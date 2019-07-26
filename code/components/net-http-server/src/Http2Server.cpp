/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HttpServer.h"
#include "HttpServerImpl.h"

#include <nghttp2/nghttp2.h>

#include <deque>

namespace net
{
class Http2Response : public HttpResponse
{
public:
	inline Http2Response(fwRefContainer<HttpRequest> request, nghttp2_session* session, int streamID)
		: HttpResponse(request), m_session(session), m_stream(streamID)
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

		if (!m_session)
		{
			return;
		}

		m_headers = headers;
		m_headers.insert({ ":status", std::to_string(statusCode) });

		for (auto& header : m_headerList)
		{
			m_headers.insert(header);
		}

		// don't have transfer_encoding at all!
		m_headers.erase("transfer-encoding");

		nghttp2_data_provider provider;
		provider.source.ptr = this;
		provider.read_callback = [](nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length, uint32_t *data_flags, nghttp2_data_source *source, void *user_data) -> ssize_t
		{
			auto resp = reinterpret_cast<Http2Response*>(source->ptr);

			if (resp->m_ended)
			{
				*data_flags = NGHTTP2_DATA_FLAG_EOF;
			}

			if (resp->m_outData.empty())
			{
				return (resp->m_ended) ? 0 : NGHTTP2_ERR_DEFERRED;
			}

			size_t toRead = std::min(length, resp->m_outData.size());

			std::copy(resp->m_outData.begin(), resp->m_outData.begin() + toRead, buf);
			resp->m_outData.erase(resp->m_outData.begin(), resp->m_outData.begin() + toRead);

			return toRead;
		};

		std::vector<nghttp2_nv> nv(m_headers.size());

		size_t i = 0;
		for (auto& hdr : m_headers)
		{
			auto& v = nv[i];

			v.flags = 0;
			v.name = (uint8_t*)hdr.first.c_str();
			v.namelen = hdr.first.length();
			v.value = (uint8_t*)hdr.second.c_str();
			v.valuelen = hdr.second.length();

			++i;
		}

		nghttp2_submit_response(m_session, m_stream, nv.data(), nv.size(), &provider);
		nghttp2_session_send(m_session);

		m_sentHeaders = true;
	}

	virtual void WriteOut(const std::vector<uint8_t>& data) override
	{
		if (m_session)
		{
			auto origSize = m_outData.size();

			m_outData.resize(m_outData.size() + data.size());
			std::copy(data.begin(), data.end(), m_outData.begin() + origSize);

			nghttp2_session_resume_data(m_session, m_stream);
			nghttp2_session_send(m_session);
		}
	}

	virtual void End() override
	{
		m_ended = true;

		if (m_session)
		{
			nghttp2_session_resume_data(m_session, m_stream);
			nghttp2_session_send(m_session);
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

		m_session = nullptr;
	}

private:
	nghttp2_session* m_session;

	int m_stream;

	HeaderMap m_headers;

	std::deque<uint8_t> m_outData;
};

Http2ServerImpl::Http2ServerImpl()
{

}

Http2ServerImpl::~Http2ServerImpl()
{

}

void Http2ServerImpl::OnConnection(fwRefContainer<TcpServerStream> stream)
{
	struct HttpRequestData;

	struct HttpConnectionData
	{
		nghttp2_session* session;

		fwRefContainer<TcpServerStream> stream;

		Http2ServerImpl* server;

		std::set<HttpRequestData*> streams;

		// cache responses independently from streams so 'clean' closes don't invalidate session
		std::list<fwRefContainer<HttpResponse>> responses;
	};

	struct HttpRequestData
	{
		HttpConnectionData* connection;

		std::map<std::string, std::string> headers;

		std::vector<uint8_t> body;

		fwRefContainer<HttpRequest> httpReq;

		fwRefContainer<HttpResponse> httpResp;
	};

	nghttp2_session_callbacks* callbacks;
	nghttp2_session_callbacks_new(&callbacks);

	nghttp2_session_callbacks_set_send_callback(callbacks, [](nghttp2_session *session, const uint8_t *data,
		size_t length, int flags, void *user_data) -> ssize_t
	{
		reinterpret_cast<HttpConnectionData*>(user_data)->stream->Write(std::vector<uint8_t>{ data, data + length });

		return length;
	});

	nghttp2_session_callbacks_set_on_begin_headers_callback(callbacks, [](nghttp2_session *session,
		const nghttp2_frame *frame,
		void *user_data)
	{
		auto conn = reinterpret_cast<HttpConnectionData*>(user_data);

		if (frame->hd.type != NGHTTP2_HEADERS ||
			frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
			return 0;
		}

		auto reqData = new HttpRequestData;
		reqData->connection = conn;
		reqData->httpReq = nullptr;
		reqData->httpResp = nullptr;

		conn->streams.insert(reqData);

		nghttp2_session_set_stream_user_data(session, frame->hd.stream_id, reqData);

		return 0;
	});

	nghttp2_session_callbacks_set_on_header_callback(callbacks, [](nghttp2_session *session,
		const nghttp2_frame *frame, const uint8_t *name,
		size_t namelen, const uint8_t *value,
		size_t valuelen, uint8_t flags,
		void *user_data) -> int
	{
		auto conn = reinterpret_cast<HttpConnectionData*>(user_data);

		switch (frame->hd.type)
		{
			case NGHTTP2_HEADERS:
				if (frame->headers.cat != NGHTTP2_HCAT_REQUEST) {
					break;
				}

				auto req = reinterpret_cast<HttpRequestData*>(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id));
				
				req->headers.insert({ {reinterpret_cast<const char*>(name), namelen}, { reinterpret_cast<const char*>(value), valuelen} });

				break;
		}

		return 0;
	});

	nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, [](nghttp2_session *session, uint8_t flags,
		int32_t stream_id, const uint8_t *data,
		size_t len, void *user_data)
	{
		auto req = reinterpret_cast<HttpRequestData*>(nghttp2_session_get_stream_user_data(session, stream_id));

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
				auto req = reinterpret_cast<HttpRequestData*>(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id));
				std::shared_ptr<HttpState> reqState = std::make_shared<HttpState>();

				if (req)
				{
					HeaderMap headerList;

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
					}

					fwRefContainer<HttpRequest> request = new HttpRequest(2, 0, req->headers[":method"], req->headers[":path"], headerList, req->connection->stream->GetPeerAddress().ToString());
					
					fwRefContainer<HttpResponse> response = new Http2Response(request, session, frame->hd.stream_id);
					req->connection->responses.push_back(response);

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
					auto req = reinterpret_cast<HttpRequestData*>(nghttp2_session_get_stream_user_data(session, frame->hd.stream_id));

					if (req->httpReq.GetRef())
					{
						auto handler = req->httpReq->GetDataHandler();
						req->httpReq->SetDataHandler();

						if (handler)
						{
							(*handler)(req->body);
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
		auto req = reinterpret_cast<HttpRequestData*>(nghttp2_session_get_stream_user_data(session, stream_id));

		auto resp = req->httpResp.GetRef();

		if (resp)
		{
			((net::Http2Response*)resp)->Cancel();
		}

		req->connection->streams.erase(req);
		delete req;

		return 0;
	});

	// create a server
	auto data = new HttpConnectionData;
	data->stream = stream;
	data->server = this;

	nghttp2_session_server_new(&data->session, callbacks, data);

	// clean callbacks
	nghttp2_session_callbacks_del(callbacks);

	// send settings
	nghttp2_settings_entry iv[1] = {
		{ NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100 } };

	nghttp2_submit_settings(data->session, NGHTTP2_FLAG_NONE, iv, 1);

	//nghttp2_session_send(data->session);

	// handle receive
	stream->SetReadCallback([=](const std::vector<uint8_t>& bytes)
	{
		nghttp2_session_mem_recv(data->session, bytes.data(), bytes.size());

		nghttp2_session_send(data->session);
	});

	stream->SetCloseCallback([=]()
	{
		{
			for (auto& response : data->responses)
			{
				// cancel HTTP responses that we have referenced
				// (so that we won't reference data->session)
				auto resp = response.GetRef();

				if (resp)
				{
					((net::Http2Response*)resp)->Cancel();
				}
			}

			data->responses.clear();
		}

		nghttp2_session_del(data->session);
		delete data;
	});


}
}
