#include "StdInc.h"
#include <ServerInstanceBase.h>
#include <HttpServerManager.h>
#include <TcpServerManager.h>

#include <ResourceManager.h>
#include <ResourceFilesComponent.h>
#include <ResourceStreamComponent.h>

#include <Client.h>
#include <ClientRegistry.h>

#include <array>
#include <filesystem>

constexpr const size_t kFileSendSize = 128 * 1024;

namespace fx
{
	struct Request
	{
		uv_fs_t fsReq;

		void HandleOpen()
		{

		}
	};

	struct UvFileHandle
	{
		inline UvFileHandle(uv_loop_t* loop, uv_file handle)
			: loop(loop), handle(handle)
		{
			req = std::make_unique<uv_fs_t>();
		}

		inline UvFileHandle(const UvFileHandle&) = delete;

		inline ~UvFileHandle()
		{
			uv_loop_t* localLoop = loop;
			uv_file localHandle = handle;

			UvCloseHelper(std::move(req), [=](auto req, auto cb)
			{
				uv_fs_close(localLoop, req, localHandle, cb);
			});
		}

		inline long get()
		{
			return handle;
		}

	private:
		uv_loop_t* loop;
		uv_file handle;

		std::unique_ptr<uv_fs_t> req;
	};

	static auto GetFilesEndpointHandler(fx::ServerInstanceBase* instance)
	{
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

		auto sendFile = [=](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response, const std::string& resourceName, const std::string& fileName, const fx::ClientSharedPtr& client)
		{
			// get resource manager and resource
			auto resourceManager = instance->GetComponent<fx::ResourceManager>();
			auto resource = resourceManager->GetResource(resourceName);

			if (!resource.GetRef())
			{
				response->SetStatusCode(404);
				response->End(fmt::sprintf("Not found. (missing requested resource: %s)", resourceName));
				return;
			}

			// get the file path
			auto filesComponent = resource->GetComponent<fx::ResourceFilesComponent>();
			auto filePairs = filesComponent->GetFileHashPairs();

			std::string fn = instance->GetRootPath() + "/cache/files/" + resourceName + "/" + fileName;

			if (filePairs.find(fileName) == filePairs.end())
			{
				// is it a stream file, instead?
				auto streamComponent = resource->GetComponent<fx::ResourceStreamComponent>();
				const auto& streamPairs = streamComponent->GetStreamingList();

				auto sit = streamPairs.find(fileName);

				if (sit == streamPairs.end())
				{
					response->SetStatusCode(404);
					response->End(fmt::sprintf("Not found. (missing requested file: %s/%s)", resourceName, fileName));
					return;
				}

				if (!sit->second.loadDiskPath.empty())
				{
					fn = sit->second.loadDiskPath;
				}
				else
				{
					fn = sit->second.onDiskPath;
				}
			}

			// get the TCP manager for a libuv loop
			fwRefContainer<net::TcpServerManager> tcpManager = instance->GetComponent<net::TcpServerManager>();
			auto uvLoop = tcpManager->GetCurrentLoop();

			// make a global request we reuse
			auto req = std::make_shared<uv_fs_t>();

			std::string fnRel = "<>";

			try
			{
				std::filesystem::path filePath(fn);
				fnRel = filePath.lexically_relative(instance->GetRootPath()).string();
			}
			catch (std::exception& e)
			{

			}

			// stat() the file
			uv_fs_stat(uvLoop, req.get(), fn.c_str(), UvCallbackWrap<uv_fs_t>(req.get(), [=](uv_fs_t* fsReq)
			{
				// failed? return failure
				if (req->result < 0)
				{
					response->SetStatusCode(500);
					response->End(fmt::sprintf("Stat of file %s failed. Error code from libuv: %s (%d)", fnRel, uv_strerror(req->result), int32_t(req->result)));
					return;
				}

				// store the size and send content-length
				auto size = fsReq->statbuf.st_size;

				// free the request
				uv_fs_req_cleanup(fsReq);

				// open the file
				uv_fs_open(uvLoop, req.get(), fn.c_str(), O_RDONLY, 0644, UvCallbackWrap<uv_fs_t>(req.get(), [=](uv_fs_t* fsReq)
				{
					// handle failure and clean up
					if (req->result < 0)
					{
						response->SetStatusCode(500);
						response->End(fmt::sprintf("Opening file %s failed. Error code from libuv: %s (%d)", fnRel, uv_strerror(req->result), int32_t(req->result)));
						return;
					}

					uv_fs_req_cleanup(fsReq);

					auto file = std::make_shared<UvFileHandle>(uvLoop, req->result);

					auto filter = filesComponent->CreateFilesFilter(fileName, request);

					std::string reason;

					if (filter && filter->ShouldTerminate(&reason))
					{
						response->SetStatusCode(403);
						response->End(fmt::sprintf("Filter failed: %s.", reason));
						return;
					}

					// write header information and a 200 OK
					response->SetHeader("content-length", std::to_string(size));
					response->SetHeader("transfer-encoding", "identity");

					response->WriteHead(200);

					// read buffer and file handle
					auto buffer = std::make_shared<std::unique_ptr<char[]>>();
					*buffer = std::unique_ptr<char[]>(new char[kFileSendSize]);

					auto uvBufRef = std::make_shared<uv_buf_t>();
					*uvBufRef = uv_buf_init(buffer->get(), kFileSendSize);

					// mutable read offset pointer
					auto readOffset = std::make_shared<size_t>(0);

					// to keep a reference to readCallback inside of itself
					auto readCallback = std::make_shared<std::function<void(uv_fs_t*)>>();
					*readCallback = [=](uv_fs_t* fsReq)
					{
						// read failed? report back
						if (fsReq->result < 0)
						{
							trace("Catastrophic write failure!\n");
							response->End();

							// reset the function reference to break the reference cycle
							*readCallback = nullptr;

							return;
						}

						// cleanup request
						uv_fs_req_cleanup(fsReq);

						// filter
						if (filter)
						{
							filter->Filter(buffer->get(), fsReq->result);
						}

						// write to response
						response->Write(std::move(*buffer), fsReq->result, [=](bool result)
						{
							if (!*readCallback)
							{
								return;
							}

							if (!result)
							{
								// if so, end response (closing should be done automatically)
								response->End();

								// reset the function reference to break the reference cycle
								*readCallback = nullptr;

								return;
							}

							// touch client
							if (client)
							{
								client->Touch();
							}

							// increment read offset
							*readOffset += req->result;

							// has the full file been read yet?
							// if not, call another read
							if (*readOffset != size)
							{
								*buffer = std::unique_ptr<char[]>(new char[kFileSendSize]);
								*uvBufRef = uv_buf_init(buffer->get(), kFileSendSize);

								uv_fs_read(uvLoop, req.get(), file->get(), uvBufRef.get(), 1, *readOffset, UvCallbackWrap<uv_fs_t>(req.get(), *readCallback));
							}
							else
							{
								// if so, end response (closing should be done automatically)
								response->End();

								// reset the function reference to break the reference cycle
								*readCallback = nullptr;
							}
						});
					};

					// on-close handler
					request->SetCancelHandler([=]()
					{
						// if so, end response (closing should be done automatically)
						response->End();

						// reset the function reference to break the reference cycle
						*readCallback = nullptr;
					});

					// trigger the first read
					uv_fs_read(uvLoop, req.get(), file->get(), uvBufRef.get(), 1, *readOffset, UvCallbackWrap<uv_fs_t>(req.get(), *readCallback));
				}));
			}));
		};

		return [=](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
		{
			if (request->GetRequestMethod() != "GET")
			{
				response->WriteHead(405);
				response->End("/files is GET only");
				return;
			}

			auto ra = request->GetRemoteAddress();
			auto token = request->GetHeader("X-CitizenFX-Token");

			fx::ClientSharedPtr client;

			if (!token.empty())
			{
				client = clientRegistry->GetClientByConnectionToken(token);
			}

			if (!client)
			{
				client = clientRegistry->GetClientByTcpEndPoint(ra.substr(0, ra.find_last_of(':')));
			}

			auto path = std::string{ request->GetPath().c_str(), request->GetPath().size() };

			if (path.length() >= 8)
			{
				std::string filesPath = path.substr(7);
				int resourceEnd = filesPath.find('/');

				if (resourceEnd != std::string::npos)
				{
					// strip any / to get the filename
					std::string resourceName = filesPath.substr(0, resourceEnd);

					do
					{
						filesPath = filesPath.substr(resourceEnd + 1);
						resourceEnd = filesPath.find('/');
					} while (resourceEnd != std::string::npos);

					std::string filesPathDecoded;
					UrlDecode(filesPath, filesPathDecoded);

					auto queryOffset = filesPathDecoded.rfind('?');
					if (queryOffset != std::string::npos)
					{
						filesPathDecoded = filesPathDecoded.substr(0, queryOffset);
					}

					if (client)
					{
						client->Touch();
					}

					sendFile(request, response, resourceName, filesPathDecoded, client);
					return;
				}
			}

			response->WriteHead(400, { { "Content-Length", "0" } });
			response->End();
		};
	}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/files", fx::GetFilesEndpointHandler(instance));
	});
});
