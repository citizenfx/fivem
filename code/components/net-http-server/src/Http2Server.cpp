/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HttpServer.h"
#include "HttpServerImpl.h"

#include <EASTL/fixed_vector.h>
#include <nghttp2/nghttp2.h>

#include <deque>

struct nghttp2_session_wrap
{
	nghttp2_session* session;

	nghttp2_session_wrap(nghttp2_session* session)
		: session(session)
	{
	
	}

	~nghttp2_session_wrap()
	{
		if (session)
		{
			nghttp2_session_del(session);
			session = {};
		}
	}

	inline operator nghttp2_session*() const
	{
		return session;
	}
};

struct ZeroCopyByteBuffer
{
	struct Element
	{
		std::string string;
		std::vector<uint8_t> vec;
		std::unique_ptr<char[]> raw;
		size_t rawLength = 0;
		size_t read = 0;

		fu2::unique_function<void(bool)> cb;

		int type;

		Element(std::vector<uint8_t>&& vec, fu2::unique_function<void(bool)>&& cb)
			: type(1), vec(std::move(vec)), cb(std::move(cb))
		{
			
		}

		Element(std::string&& str, fu2::unique_function<void(bool)>&& cb)
			: type(0), string(std::move(str)), cb(std::move(cb))
		{
			
		}

		Element(std::unique_ptr<char[]> raw, size_t length, fu2::unique_function<void(bool)>&& cb)
			: type(2), raw(std::move(raw)), rawLength(length), cb(std::move(cb))
		{

		}

		// we lied! we copy anyway :(
		Element(const std::vector<uint8_t>& vec, fu2::unique_function<void(bool)>&& cb)
			: type(1), vec(vec), cb(std::move(cb))
		{
			this->vec = vec;
		}

		Element(const std::string& str, fu2::unique_function<void(bool)>&& cb)
			: type(0), string(str), cb(std::move(cb))
		{
			
		}

		size_t Size() const
		{
			switch (type)
			{
			case 0:
				return string.size();
			case 1:
				return vec.size();
			case 2:
				return rawLength;
			}

			return 0;
		}

		void Advance(size_t len)
		{
			read += len;
		}

		size_t Leftover() const
		{
			return Size() - read;
		}
	};

	template<typename TContainer>
	void Push(TContainer&& elem, fu2::unique_function<void(bool)>&& cb)
	{
		elements.emplace_back(std::move(elem), std::move(cb));
	}

	void Push(std::unique_ptr<char[]> data, size_t size, fu2::unique_function<void(bool)>&& cb)
	{
		elements.emplace_back(std::move(data), size, std::move(cb));
	}

	bool Pop(const std::string** str, const std::vector<uint8_t>** vec, size_t* size, size_t* off)
	{
		if (elements.empty())
		{
			return false;
		}

		const auto& elem = elements.front();
		*off = elem.read;

		switch (elem.type)
		{
		case 0:
			*str = &elem.string;
			*vec = nullptr;
			break;
		case 1:
			*vec = &elem.vec;
			*str = nullptr;
			break;
		case 2:
			*vec = nullptr;
			*str = nullptr;
			*size = elem.rawLength;
			break;
		}

		return true;
	}

	ssize_t PeekLength()
	{
		const std::string* s;
		const std::vector<uint8_t>* v;
		size_t size = 0;
		size_t off = 0;

		if (Pop(&s, &v, &size, &off))
		{
			if (s)
			{
				return s->size() - off;
			}
			else if (v)
			{
				return v->size() - off;
			}
			else if (size)
			{
				return size - off;
			}
		}

		return -1;
	}

	bool Take(uint32_t length, std::string* str, std::vector<uint8_t>* vec, std::unique_ptr<char[]>* raw, size_t* rawLength, size_t* off, fu2::unique_function<void(bool)>* cb)
	{
		if (elements.empty())
		{
			return false;
		}

		{
			auto& elem = elements.front();
			*off = elem.read;

			switch (elem.type)
			{
				case 0:
				{
					if (length < elem.Leftover())
					{
						*raw = std::unique_ptr<char[]>(new char[length]);
						*rawLength = length;
						*off = 0;

						memcpy(raw->get(), elem.string.data() + elem.read, length);
						elem.Advance(length);

						return true;
					}

					*str = std::move(elem.string);
					break;
				}
				case 1:
				{
					if (length < elem.Leftover())
					{
						*raw = std::unique_ptr<char[]>(new char[length]);
						*rawLength = length;
						*off = 0;

						memcpy(raw->get(), elem.vec.data() + elem.read, length);
						elem.Advance(length);

						return true;
					}

					*vec = std::move(elem.vec);
					break;
				}
				case 2:
				{
					if (length < elem.Leftover())
					{
						*raw = std::unique_ptr<char[]>(new char[length]);
						*rawLength = length;
						*off = 0;

						memcpy(raw->get(), elem.raw.get() + elem.read, length);
						elem.Advance(length);

						return true;
					}

					*raw = std::move(elem.raw);
					*rawLength = std::move(elem.rawLength);
					break;
				}
			}

			*cb = std::move(elem.cb);
		}

		elements.pop_front();

		return true;
	}

	bool Has(size_t len)
	{
		size_t accounted = 0;

		for (const auto& elem : elements)
		{
			auto thisLen = std::min(len - accounted, elem.Size() - elem.read);
			accounted += thisLen;

			if (accounted > len)
			{
				return true;
			}
		}

		return (len <= accounted);
	}

	void Advance(size_t len)
	{
		while (len > 0)
		{
			auto& elem = elements.front();
			size_t s = elem.Size();

			auto thisLen = std::min(len, s - elem.read);
			elem.read += thisLen;
			len -= thisLen;

			if (elem.read >= s)
			{
				elements.pop_front();
			}
		}
	}

	bool Empty()
	{
		return elements.empty();
	}

private:
	std::deque<Element> elements;
};

namespace net
{
class Http2Response : public HttpResponse
{
public:
	inline Http2Response(fwRefContainer<HttpRequest> request, const std::shared_ptr<nghttp2_session_wrap>& session, int streamID, const fwRefContainer<net::TcpServerStream>& tcpStream)
		: HttpResponse(request), m_session(session), m_stream(streamID), m_tcpStream(tcpStream)
	{

	}

	virtual ~Http2Response()
	{

	}

	virtual void WriteHead(int statusCode, const std::string& statusMessage, const HeaderMap& headers) override
	{
		if (m_sentHeaders)
		{
			return;
		}

		auto session = Session();

		if (!session)
		{
			return;
		}

		auto statusCodeStr = std::to_string(statusCode);

		m_headers.emplace_back(":status", HeaderString{ statusCodeStr.c_str(), statusCodeStr.size() });

		auto addHeader = [this](const auto& header)
		{
			// don't have transfer_encoding at all!
			if (_stricmp(header.first.c_str(), "transfer-encoding") != 0)
			{
				m_headers.push_back(header);
			}
		};

		for (const auto& header : headers)
		{
			addHeader(header);
		}

		for (const auto& header : m_headerList)
		{
			addHeader(header);
		}

		if (auto stream = TcpStream(); stream.GetRef())
		{
			fwRefContainer thisRef = this;

			stream->ScheduleCallback([thisRef, session]()
			{
				nghttp2_data_provider provider;
				provider.source.ptr = thisRef.GetRef();
				provider.read_callback = [](nghttp2_session* session, int32_t stream_id, uint8_t* buf, size_t length, uint32_t* data_flags, nghttp2_data_source* source, void* user_data) -> ssize_t
				{
					auto resp = reinterpret_cast<Http2Response*>(source->ptr);

					if (resp->m_ended)
					{
						*data_flags = NGHTTP2_DATA_FLAG_EOF;
					}

					if (resp->m_buffer.Empty())
					{
						return (resp->m_ended) ? 0 : NGHTTP2_ERR_DEFERRED;
					}

					*data_flags |= NGHTTP2_DATA_FLAG_NO_COPY;
					return std::min(size_t(resp->m_buffer.PeekLength()), length);
				};

				std::vector<nghttp2_nv> nv(thisRef->m_headers.size());

				size_t i = 0;
				for (auto& hdr : thisRef->m_headers)
				{
					auto& v = nv[i];

					v.flags = 0;
					v.name = (uint8_t*)hdr.first.c_str();
					v.namelen = hdr.first.length();
					v.value = (uint8_t*)hdr.second.c_str();
					v.valuelen = hdr.second.length();

					++i;
				}
				nghttp2_submit_response(*session, thisRef->m_stream, nv.data(), nv.size(), &provider);
				nghttp2_session_send(*session);

				thisRef->m_sentHeaders = true;
			});
		}
	}

	template<typename TContainer>
	void WriteOutInternal(TContainer data, fu2::unique_function<void(bool)> && cb = {})
	{
		if (auto stream = TcpStream(); stream.GetRef())
		{
			fwRefContainer thisRef = this;

			stream->ScheduleCallback([thisRef, data = std::move(data), cb = std::move(cb)]() mutable
			{
				if (auto session = thisRef->Session())
				{
					thisRef->m_buffer.Push(std::forward<TContainer>(data), std::move(cb));

					nghttp2_session_resume_data(*session, thisRef->m_stream);
					nghttp2_session_send(*session);
				}
			});
		}
	}

	virtual void WriteOut(const std::vector<uint8_t>& data, fu2::unique_function<void(bool)>&& cb = {}) override
	{
		WriteOutInternal<decltype(data)>(data, std::move(cb));
	}

	virtual void WriteOut(std::vector<uint8_t>&& data, fu2::unique_function<void(bool)>&& cb = {}) override
	{
		WriteOutInternal<decltype(data)>(std::move(data), std::move(cb));
	}

	virtual void WriteOut(const std::string& data, fu2::unique_function<void(bool)>&& cb = {}) override
	{
		WriteOutInternal<decltype(data)>(data, std::move(cb));
	}

	virtual void WriteOut(std::string&& data, fu2::unique_function<void(bool)>&& cb = {}) override
	{
		WriteOutInternal<decltype(data)>(std::move(data), std::move(cb));
	}

	virtual void WriteOut(std::unique_ptr<char[]> data, size_t size, fu2::unique_function<void(bool)>&& cb = {}) override
	{
		if (auto stream = TcpStream(); stream.GetRef())
		{
			fwRefContainer thisRef = this;

			stream->ScheduleCallback([thisRef, data = std::move(data), size, cb = std::move(cb)]() mutable
			{
				if (auto session = thisRef->Session())
				{
					thisRef->m_buffer.Push(std::move(data), size, std::move(cb));

					nghttp2_session_resume_data(*session, thisRef->m_stream);
					nghttp2_session_send(*session);
				}
			});
		}
	}

	virtual void End() override
	{
		if (auto stream = TcpStream(); stream.GetRef())
		{
			fwRefContainer thisRef = this;

			stream->ScheduleCallback([thisRef]()
			{
				thisRef->m_tcpStream = nullptr;

				auto session = thisRef->Session();
				thisRef->m_ended = true;

				if (session)
				{
					nghttp2_session_resume_data(*session, thisRef->m_stream);
					nghttp2_session_send(*session);
				}
			});
		}
		else
		{
			m_ended = true;
		}
	}

	virtual void CloseSocket() override
	{
		m_ended = true;

		if (auto s = TcpStream(); s.GetRef())
		{
			s->Close();
		}
	}

	void Cancel()
	{
		if (m_request.GetRef() && !m_ended)
		{
			auto cancelHandler = m_request->GetCancelHandler();

			if (cancelHandler)
			{
				(*cancelHandler)();

				m_request->SetCancelHandler();
			}
		}

		TcpStream({});
		Session({});
	}

	inline ZeroCopyByteBuffer& GetBuffer()
	{
		return m_buffer;
	}

private:
	std::shared_ptr<nghttp2_session_wrap> Session()
	{
		std::shared_lock _(m_refMutex);
		return m_session;
	}

	void Session(const std::shared_ptr<nghttp2_session_wrap>& session)
	{
		std::unique_lock _(m_refMutex);
		m_session = session;
	}

	fwRefContainer<net::TcpServerStream> TcpStream()
	{
		std::shared_lock _(m_refMutex);
		return m_tcpStream;
	}

	void TcpStream(const fwRefContainer<net::TcpServerStream>& tcpStream)
	{
		std::unique_lock _(m_refMutex);
		m_tcpStream = tcpStream;
	}

private:
	std::shared_ptr<nghttp2_session_wrap> m_session;

	std::shared_mutex m_refMutex;

	int m_stream;

	eastl::fixed_vector<eastl::pair<HeaderString, HeaderString>, 16> m_headers;

	ZeroCopyByteBuffer m_buffer;

	fwRefContainer<net::TcpServerStream> m_tcpStream;
};

namespace h2
{
	class HttpRequestData;

	struct HttpConnectionData
	{
		std::shared_ptr<nghttp2_session_wrap> session;

		fwRefContainer<TcpServerStream> stream;

		Http2ServerImpl* server = nullptr;

		std::set<fwRefContainer<HttpRequestData>> streams;

		// cache responses independently from streams so 'clean' closes don't invalidate session
		std::set<fwRefContainer<HttpResponse>> responses;
		std::shared_mutex responsesMutex;
	};

	class HttpRequestData : public fwRefCountable
	{
	public:
		HttpConnectionData* connection = nullptr;

		HeaderMap headers;

		std::vector<uint8_t> body;

		fwRefContainer<HttpRequest> httpReq;

		fwRefContainer<HttpResponse> httpResp;

		virtual ~HttpRequestData() override = default;
	};
}

void Http2ServerImpl::OnConnection(fwRefContainer<TcpServerStream> stream)
{
	nghttp2_session_callbacks* callbacks;
	nghttp2_session_callbacks_new(&callbacks);

	nghttp2_session_callbacks_set_send_callback(callbacks, [](nghttp2_session *session, const uint8_t *data,
		size_t length, int flags, void *user_data) -> ssize_t
	{
		reinterpret_cast<h2::HttpConnectionData*>(user_data)->stream->Write(std::vector<uint8_t>{ data, data + length });

		return length;
	});

	nghttp2_session_callbacks_set_send_data_callback(callbacks, [](nghttp2_session* session, nghttp2_frame* frame, const uint8_t* framehd, size_t length, nghttp2_data_source* source, void* user_data) -> int
	{
		auto data = reinterpret_cast<h2::HttpConnectionData*>(user_data);
		auto resp = reinterpret_cast<Http2Response*>(source->ptr);

		auto& buf = resp->GetBuffer();

		if (buf.Has(length))
		{
			static thread_local std::vector<uint8_t> fhd(9);
			memcpy(&fhd[0], framehd, fhd.size());

			data->stream->Write(fhd);

			std::vector<uint8_t> v;
			std::string s;
			std::unique_ptr<char[]> raw;
			size_t rawLength;
			size_t off;
			fu2::unique_function<void(bool)> cb;

			if (buf.Take(static_cast<uint32_t>(length), &s, &v, &raw, &rawLength, &off, &cb))
			{
				if (off == 0)
				{
					if (!s.empty())
					{
						data->stream->Write(std::move(s), std::move(cb));
					}
					else if (!v.empty())
					{
						data->stream->Write(std::move(v), std::move(cb));
					}
					else if (raw)
					{
						data->stream->Write(std::move(raw), rawLength, std::move(cb));
					}
				}
				else
				{
					if (!s.empty())
					{
						auto d = std::unique_ptr<char[]>(new char[length]);
						memcpy(d.get(), s.data() + off, length);

						data->stream->Write(std::move(d), length, std::move(cb));
					}
					else if (!v.empty())
					{
						auto d = std::unique_ptr<char[]>(new char[length]);
						memcpy(d.get(), v.data() + off, length);

						data->stream->Write(std::move(d), length, std::move(cb));
					}
					else if (raw)
					{
						auto d = std::unique_ptr<char[]>(new char[length]);
						memcpy(d.get(), raw.get() + off, length);

						data->stream->Write(std::move(d), length, std::move(cb));
					}
				}
			}

			return 0;
		}

		return NGHTTP2_ERR_WOULDBLOCK;
	});

	nghttp2_session_callbacks_set_on_begin_headers_callback(callbacks, [](nghttp2_session *session,
		const nghttp2_frame *frame,
		void *user_data)
	{
		auto conn = reinterpret_cast<h2::HttpConnectionData*>(user_data);

		if (frame->hd.type != NGHTTP2_HEADERS ||
			frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
			return 0;
		}

		fwRefContainer reqData = new h2::HttpRequestData;
		reqData->connection = conn;
		reqData->httpReq = nullptr;
		reqData->httpResp = nullptr;

		conn->streams.insert(reqData);

		nghttp2_session_set_stream_user_data(session, frame->hd.stream_id, reqData.GetRef());

		return 0;
	});

	nghttp2_session_callbacks_set_on_header_callback(callbacks, [](nghttp2_session *session,
		const nghttp2_frame *frame, const uint8_t *name,
		size_t namelen, const uint8_t *value,
		size_t valuelen, uint8_t flags,
		void *user_data) -> int
	{
		auto conn = reinterpret_cast<h2::HttpConnectionData*>(user_data);

		switch (frame->hd.type)
		{
			case NGHTTP2_HEADERS:
				if (frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
					break;
				}

				auto req = reinterpret_cast<h2::HttpRequestData*>(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id));
				
				req->headers.insert({ {reinterpret_cast<const char*>(name), namelen}, { reinterpret_cast<const char*>(value), valuelen} });

				break;
		}

		return 0;
	});

	nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, [](nghttp2_session *session, uint8_t flags,
		int32_t stream_id, const uint8_t *data,
		size_t len, void *user_data)
	{
		auto req = reinterpret_cast<h2::HttpRequestData*>(nghttp2_session_get_stream_user_data(session, stream_id));

		if (req)
		{
			size_t origSize = req->body.size();

			req->body.resize(origSize + len);
			memcpy(&req->body[origSize], data, len);
		}

		return 0;
	});

	nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, [](nghttp2_session *session,
		const nghttp2_frame *frame, void *user_data)
	{
		switch (frame->hd.type) {
			case NGHTTP2_HEADERS:
			{
				auto req = reinterpret_cast<h2::HttpRequestData*>(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id));
				std::shared_ptr<HttpState> reqState = std::make_shared<HttpState>();

				if (req)
				{
					HeaderMap headerList;
					HeaderString method;
					HeaderString path;

					for (auto& header : req->headers)
					{
						if (header.first[0] != ':')
						{
							headerList.insert(header);
						}
						else if (header.first == ":authority")
						{
							headerList.emplace("host", header.second);
						}
						else if (header.first == ":method")
						{
							method = header.second;
						}
						else if (header.first == ":path")
						{
							path = header.second;
						}
					}

					fwRefContainer<HttpRequest> request = new HttpRequest(2, 0, method, path, headerList, req->connection->stream->GetPeerAddress());
					
					fwRefContainer<HttpResponse> response = new Http2Response(request, req->connection->session, frame->hd.stream_id, req->connection->stream);

					{
						std::unique_lock _(req->connection->responsesMutex);
						req->connection->responses.insert(response);
					}

					req->httpResp = response;
					reqState->blocked = true;

					for (auto& handler : req->connection->server->m_handlers)
					{
						if (handler->HandleRequest(request, response) || response->HasEnded())
						{
							break;
						}
					}

					if (!response->HasEnded())
					{
						req->httpReq = request;
					}
				}
			}
			break;
			case NGHTTP2_DATA:
				if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM)
				{
					auto req = reinterpret_cast<h2::HttpRequestData*>(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id));

					if (req->httpReq.GetRef())
					{
						auto handler = req->httpReq->GetDataHandler();
						req->httpReq->SetDataHandler();

						if (handler)
						{
							(*handler)(req->body);
						}
						else
						{
							req->httpReq->SetPendingData(std::move(req->body));
						}
					}
				}

				break;
		}

		return 0;
	});

	nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, [](nghttp2_session *session, int32_t stream_id,
		uint32_t error_code, void *user_data)
	{
		auto req = reinterpret_cast<h2::HttpRequestData*>(nghttp2_session_get_stream_user_data(session, stream_id));

		auto resp = req->httpResp.GetRef();

		if (resp)
		{
			((net::Http2Response*)resp)->Cancel();

			std::unique_lock _(req->connection->responsesMutex);
			req->connection->responses.erase(resp);
		}

		req->connection->streams.erase(req);

		return 0;
	});

	// create a server
	auto data = std::make_shared<h2::HttpConnectionData>();
	data->stream = stream;
	data->server = this;

	nghttp2_session* session;
	nghttp2_session_server_new(&session, callbacks, data.get());

	data->session = std::make_shared<nghttp2_session_wrap>(session);

	// clean callbacks
	nghttp2_session_callbacks_del(callbacks);

	// send settings
	nghttp2_settings_entry iv[1] = {
		{ NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100 } };

	nghttp2_submit_settings(*data->session, NGHTTP2_FLAG_NONE, iv, 1);

	//nghttp2_session_send(data->session);

	// handle receive
	stream->SetReadCallback([data](const std::vector<uint8_t>& bytes)
	{
		nghttp2_session_mem_recv(*data->session, bytes.data(), bytes.size());

		nghttp2_session_send(*data->session);
	});

	stream->SetCloseCallback([data]()
	{
		{
			decltype(data->responses) responsesCopy;

			{
				std::shared_lock _(data->responsesMutex);
				responsesCopy = data->responses;
			}

			for (auto& response : responsesCopy)
			{
				// cancel HTTP responses that we have referenced
				// (so that we won't reference data->session)
				auto resp = response.GetRef();

				if (resp)
				{
					((net::Http2Response*)resp)->Cancel();
				}
			}

			std::unique_lock _(data->responsesMutex);
			data->responses.clear();
		}

		// free any leftover stream data
		data->streams.clear();
	});
}
}
