#include <StdInc.h>

#ifdef GTA_FIVE
#include <LegitimacyAPI.h>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <json.hpp>

#include <skyr/url.hpp>

#include <HttpClient.h>

using client = websocketpp::client<websocketpp::config::asio_client>;
using message_ptr = websocketpp::config::asio_client::message_type::ptr;

constexpr auto MIN_PORT = 6463;
constexpr auto MAX_PORT = 6472;

using json = nlohmann::json;

static HookFunction initFunction([]()
{
	std::thread([]()
	{
		try
		{
			client c;
			c.init_asio();

			static int curPort;

			auto try_port = [&c](int port)
			{
				curPort = port;

				websocketpp::lib::error_code ec;
				auto con = c.get_connection(fmt::sprintf("ws://127.0.0.1:%d/?v=1", port), ec);

				if (!ec)
				{
					// this enables top secret internal RPC mode
					con->append_header("origin", "https://discordapp.com");
					c.connect(con);
				}
			};

			c.set_fail_handler([&try_port](websocketpp::connection_hdl hdl)
			{
				if (curPort == MAX_PORT)
				{
					return;
				}

				try_port(curPort + 1);
			});

			c.set_message_handler([&c](websocketpp::connection_hdl hdl, message_ptr msg)
			{
				auto obj = json::parse(msg->get_payload());

				auto cmd = obj.value("cmd", "");
				if (cmd == "DISPATCH")
				{
					auto evt = obj.value("evt", "");

					if (evt == "READY")
					{
						// send overlay handshake
						auto overlaySubscribe = json::object({
							{"cmd", "SUBSCRIBE"},
							{"args", json::object()},
							{"evt", "OVERLAY"},
							{"nonce", "nonce"}
						});

						auto overlayInit = json::object({
							{"cmd", "OVERLAY"},
							{"args", json::object(
								{
									{"type", "CONNECT"},
									{"pid", -1}
								}
							)},
							{"nonce", "nonce2"}
						});

						websocketpp::lib::error_code ec;
						c.send(hdl, overlaySubscribe.dump(), websocketpp::frame::opcode::text, ec);
						c.send(hdl, overlayInit.dump(), websocketpp::frame::opcode::text, ec);
					}
					else if (evt == "OVERLAY")
					{
						auto& data = obj["data"];

						if (data["type"] == "DISPATCH")
						{
							auto& payloads = data["payloads"];

							for (auto& payload : payloads)
							{
								if (payload["type"] == "OVERLAY_INITIALIZE")
								{
									auto token = payload["token"].get<std::string>();

									auto authRequest = json::object({
										{ "permissions", 0 },
										{ "authorize", true }
									});

									c.close(hdl, 0, "OK");

									HttpRequestOptions options;
									options.headers.insert({ "Content-Type", "application/json" });
									options.headers.insert({ "Authorization", token });

									Instance<HttpClient>::Get()->DoPostRequest(
										"https://discordapp.com/api/v6/oauth2/authorize?client_id=530784411080458270&response_type=code&scope=identify",
										authRequest.dump(),
										options,
										[](bool result, const char* data, size_t len)
									{
										if (result)
										{
											try
											{
												auto obj = json::parse(std::string(data, len));
												auto location = obj.value("location", "");

												if (!location.empty())
												{
													auto uri = skyr::make_url(location);
													
													if (uri)
													{
														auto queryStr = uri->search();
														queryStr = queryStr.substr(queryStr.find_first_of("=") + 1);

														Instance<::HttpClient>::Get()->DoPostRequest(
															"https://lambda.fivem.net/api/validate/discord",
															{
																{ "entitlementId", ros::GetEntitlementSource() },
																{ "authCode", queryStr }
															},
															[](bool, const char*, size_t)
														{

														});
													}
												}
											}
											catch (std::exception& e)
											{

											}
										}
									});
								}
							}
						}
					}
				}
			});

			try_port(MIN_PORT);

			c.run();
		}
		catch (websocketpp::exception const& e)
		{
			trace("discord auth failed: %s\n", e.what());
		}
	}).detach();
});
#endif
