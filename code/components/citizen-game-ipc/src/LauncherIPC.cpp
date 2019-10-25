#include <StdInc.h>
#include <LauncherIPC.h>

#include <nngpp/protocol/pair1.h>

namespace ipc
{
	struct rpc_msg
	{
		std::string name;
		std::vector<msgpack::object> args;

		MSGPACK_DEFINE_ARRAY(name, args);
	};

	Endpoint::Endpoint(const std::string& bindingPrefix, bool server)
	{
		if (server)
		{
			repSock = nng::pair::open();
			repSock.listen(("ipc://cfx_sv_" + bindingPrefix).c_str());
		}
		else
		{
			try
			{
				repSock = nng::pair::open();
				repSock.dial(("ipc://cfx_sv_" + bindingPrefix).c_str());
			}
			catch (nng::exception& e)
			{
				repSock = {};
			}
		}
	}

	void Endpoint::RunFrame()
	{
		if (!repSock)
		{
			return;
		}

		while (true)
		{
			auto msg = repSock.recv(nng::flag::nonblock);

			if (msg.size() == 0)
			{
				break;
			}

			msgpack::unpacked u = msgpack::unpack((const char*)msg.data(), msg.size());
			auto o = u.get().as<rpc_msg>();

			auto handler = handlers.find(o.name);

			bool meh = false;

			if (handler != handlers.end())
			{
				msgpack::sbuffer rb;

				if (handler->second(o.args, rb))
				{
					if (rb.size() > 0)
					{
						auto buffer = nng::make_buffer(rb.size());
						memcpy(buffer.data(), rb.data(), rb.size());

						//repSock.send(std::move(buffer));
					}
					else
					{
						//repSock.send(nng::view{ &meh, 0 });
					}
				}
				else
				{
					//repSock.send(nng::view{ &meh, 0 });
				}
			}
			else
			{
				//repSock.send(nng::view{ &meh, 0 });
			}
		}
	}

	void Endpoint::Call(const void* data, size_t size)
	{
		if (!repSock)
		{
			return;
		}

		repSock.send(nng::view{ data, size }, nng::flag::nonblock);
	}
}
