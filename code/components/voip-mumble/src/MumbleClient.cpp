/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleClientImpl.h"
#include <thread>
#include <chrono>

#include <json.hpp>

#include <UvLoopManager.h>

#include <CoreConsole.h>

using json = nlohmann::json;

#include "PacketDataStream.h"

static __declspec(thread) MumbleClient* g_currentMumbleClient;

using namespace std::chrono_literals;

inline std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

void MumbleClient::Initialize()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	m_voiceTarget = 0;

	m_lastUdp = {};
	m_nextPing = {};

	m_loop = Instance<net::UvLoopManager>::Get()->GetOrCreate("mumble");

	m_loop->EnqueueCallback([this]()
	{
		m_connectTimer = m_loop->Get()->resource<uvw::TimerHandle>();
		m_connectTimer->on<uvw::TimerEvent>([this](const uvw::TimerEvent& ev, uvw::TimerHandle& t)
		{
			if (m_connectionInfo.isConnecting)
			{
				return;
			}

			m_connectionInfo.isConnecting = true;

			if (m_tcp)
			{
				m_tcp->shutdown();
				m_tcp->close();
			}

			m_tcp = m_loop->Get()->resource<uvw::TCPHandle>();

			m_tcp->on<uvw::ConnectEvent>([this](const uvw::ConnectEvent& ev, uvw::TCPHandle& tcp)
			{
				m_handler.Reset();

				m_connectionInfo.isConnecting = false;

				try
				{
					m_sessionManager = std::make_unique<Botan::TLS::Session_Manager_In_Memory>(m_rng);

					m_credentials = std::make_unique<MumbleCredentialsManager>();

					m_tcp->read();

					m_tlsClient = std::make_shared<Botan::TLS::Client>(*this,
						*(m_sessionManager.get()),
						*(m_credentials.get()),
						m_policy,
						m_rng,
						Botan::TLS::Server_Information()
						);
				}
				catch (std::exception& e)
				{
					trace("Mumble exception: %s\n", e.what());
				}

				// don't start idle timer here - it should only start after TLS handshake is done!

				m_connectionInfo.isConnected = true;
			});

			m_tcp->on<uvw::ErrorEvent>([this](const uvw::ErrorEvent& ev, uvw::TCPHandle& tcp)
			{
				console::DPrintf("Mumble", "connecting failed: %s\n", ev.what());

				m_connectionInfo.isConnecting = false;
				m_idleTimer->start(2s, 500ms);

				m_connectionInfo.isConnected = false;
			});

			m_tcp->on<uvw::EndEvent>([this](const uvw::EndEvent& ev, uvw::TCPHandle& tcp)
			{
				// TCP close, graceful?
				console::DPrintf("Mumble", "TCP close.\n");

				m_connectionInfo.isConnecting = false;
				m_idleTimer->start(2s, 500ms);

				m_connectionInfo.isConnected = false;
			});

			m_tcp->on<uvw::DataEvent>([this](const uvw::DataEvent& ev, uvw::TCPHandle& tcp)
			{
				try
				{
					if (ev.length > 0)
					{
						std::unique_lock<std::recursive_mutex> lock(m_clientMutex);
						m_tlsClient->received_data(reinterpret_cast<uint8_t*>(ev.data.get()), ev.length);
					}
				}
				catch (std::exception& e)
				{
					trace("Mumble exception: %s\n", e.what());
				}
			});

			if (!m_udp)
			{
				m_udp = m_loop->Get()->resource<uvw::UDPHandle>();

				m_udp->on<uvw::UDPDataEvent>([this](const uvw::UDPDataEvent& ev, uvw::UDPHandle& udp)
				{
					std::unique_lock<std::recursive_mutex> lock(m_clientMutex);

					try
					{
						HandleUDP(reinterpret_cast<const uint8_t*>(ev.data.get()), ev.length);
					}
					catch (std::exception& e)
					{
						trace("Mumble exception: %s\n", e.what());
					}
				});

				m_udp->recv();
			}

			const auto& address = m_connectionInfo.address;
			m_tcp->connect(*address.GetSocketAddress());
			m_state.Reset();
			m_state.SetClient(this);
			m_state.SetUsername(ToWide(m_connectionInfo.username));
		});

		m_idleTimer = m_loop->Get()->resource<uvw::TimerHandle>();
		m_idleTimer->on<uvw::TimerEvent>([this](const uvw::TimerEvent& ev, uvw::TimerHandle& t)
		{
			if (m_tlsClient && m_tlsClient->is_active() && m_connectionInfo.isConnected)
			{
				std::lock_guard<std::recursive_mutex> l(m_clientMutex);

				if (m_curManualChannel != m_lastManualChannel && !m_state.GetChannels().empty())
				{
					// check if the channel already exists
					std::wstring wname = ToWide(m_curManualChannel);
					m_lastManualChannel = m_curManualChannel;

					bool existed = false;

					for (const auto& channel : m_state.GetChannels())
					{
						if (channel.second.GetName() == wname)
						{
							// join the channel
							MumbleProto::UserState state;
							state.set_session(m_state.GetSession());
							state.set_channel_id(channel.first);

							Send(MumbleMessageType::UserState, state);
							existed = true;

							break;
						}
					}

					// it does not, create the channel (server will verify name matches)
					if (!existed)
					{
						MumbleProto::ChannelState chan;
						chan.set_parent(0);
						chan.set_name(m_curManualChannel);
						chan.set_temporary(true);

						Send(MumbleMessageType::ChannelState, chan);
					}
				}

				{
					std::vector<std::string> removeChannelListens;
					std::vector<std::string> addChannelListens;

					std::vector<int> removeChannelListenIds;
					std::vector<int> addChannelListenIds;

					std::set_difference(m_lastChannelListens.begin(), m_lastChannelListens.end(), m_curChannelListens.begin(), m_curChannelListens.end(), std::back_inserter(removeChannelListens));
					std::set_difference(m_curChannelListens.begin(), m_curChannelListens.end(), m_lastChannelListens.begin(), m_lastChannelListens.end(), std::back_inserter(addChannelListens));

					auto findCh = [&](const std::string& ch)
					{
						std::wstring wname = ToWide(ch);

						for (const auto& channel : m_state.GetChannels())
						{
							if (channel.second.GetName() == wname)
							{
								return channel.first;
							}
						}

						return uint32_t(-1);
					};

					for (const auto& remove : removeChannelListens)
					{
						auto ch = findCh(remove);

						if (ch != -1)
						{
							removeChannelListenIds.push_back(ch);
						}

						// we can remove this here, as it doesn't exist anymore: we don't listen to it either then
						m_lastChannelListens.erase(remove);
					}

					for (const auto& add : addChannelListens)
					{
						auto ch = findCh(add);

						if (ch != -1)
						{
							addChannelListenIds.push_back(ch);
							m_lastChannelListens.insert(add);
						}
					}

					if (!addChannelListenIds.empty() || !removeChannelListenIds.empty())
					{
						// send updated listens
						MumbleProto::UserState state;
						state.set_session(m_state.GetSession());

						for (auto id : addChannelListenIds)
						{
							state.add_listening_channel_add(id);
						}

						for (auto id : removeChannelListenIds)
						{
							state.add_listening_channel_remove(id);
						}

						Send(MumbleMessageType::UserState, state);
					}
				}

				{
					for (auto& [idx, config] : m_pendingVoiceTargetUpdates)
					{
						MumbleProto::VoiceTarget target;
						target.set_id(idx);

						for (auto& t : config.targets)
						{
							auto vt = target.add_targets();

							for (auto& userName : t.users)
							{
								m_state.ForAllUsers([this, &userName, &vt](const std::shared_ptr<MumbleUser>& user)
								{
									if (user->GetName() == userName)
									{
										vt->add_session(user->GetSessionId());
									}
								});
							}

							if (!t.channel.empty())
							{
								for (auto& channelPair : m_state.GetChannels())
								{
									if (channelPair.second.GetName() == ToWide(t.channel))
									{
										vt->set_channel_id(channelPair.first);
									}
								}
							}

							vt->set_links(t.links);
							vt->set_children(t.children);
						}

						Send(MumbleMessageType::VoiceTarget, target);
					}

					m_pendingVoiceTargetUpdates.clear();
				}

				auto self = m_state.GetUser(m_state.GetSession());

				if (self)
				{
					const auto& chList = m_state.GetChannels();
					auto it = chList.find(self->GetChannelId());

					if (it != chList.end())
					{
						const auto& name = it->second.GetName();

						if (!name.empty())
						{
							m_lastManualChannel = ToNarrow(name);
						}
					}
				}

				if (msec() > m_nextPing)
				{
					{
						MumbleProto::Ping ping;
						ping.set_timestamp(msec().count());
						ping.set_tcp_ping_avg(m_tcpPingAverage);
						ping.set_tcp_ping_var(m_tcpPingVariance);
						ping.set_tcp_packets(m_tcpPingCount);

						ping.set_udp_ping_avg(m_udpPingAverage);
						ping.set_udp_ping_var(m_udpPingVariance);
						ping.set_udp_packets(m_udpPingCount);

						Send(MumbleMessageType::Ping, ping);
					}

					{
						char pingBuf[64] = { 0 };

						PacketDataStream pds(pingBuf, sizeof(pingBuf));
						pds.append((1 << 5));
						pds << uint64_t(msec().count());

						SendUDP(pingBuf, pds.size());
					}

					m_nextPing = msec() + 2500ms;
				}
			}
			else
			{
				console::DPrintf("Mumble", "Reconnecting.\n");

				m_connectTimer->start(2500ms, 0s);
				m_idleTimer->stop();
			}
		});
	});

	m_audioInput.Initialize();
	m_audioInput.SetClient(this);

	m_audioOutput.Initialize();
	m_audioOutput.SetClient(this);
}

concurrency::task<MumbleConnectionInfo*> MumbleClient::ConnectAsync(const net::PeerAddress& address, const std::string& userName)
{
	m_connectionInfo.address = address;
	m_connectionInfo.username = userName;

	m_curManualChannel = "Root";

	m_tcpPingAverage = 0.0f;
	m_tcpPingVariance = 0.0f;

	m_tcpPingCount = 0;

	m_lastUdp = {};

	memset(m_tcpPings, 0, sizeof(m_tcpPings));

	m_state.SetClient(this);
	m_state.SetUsername(ToWide(userName));

	m_loop->EnqueueCallback([this]()
	{
		m_connectTimer->start(50ms, 0s);
	});

	m_completionEvent = concurrency::task_completion_event<MumbleConnectionInfo*>();

	return concurrency::task<MumbleConnectionInfo*>(m_completionEvent);
}

concurrency::task<void> MumbleClient::DisconnectAsync()
{
	if (m_tlsClient)
	{
		m_tlsClient->close();
	}

	m_loop->EnqueueCallback([this]()
	{
		if (m_idleTimer)
		{
			m_idleTimer->stop();
		}
	});

	m_connectionInfo = {};

	return concurrency::task_from_result();
}

void MumbleClient::SetActivationMode(MumbleActivationMode mode)
{
	return m_audioInput.SetActivationMode(mode);
}

void MumbleClient::SetActivationLikelihood(MumbleVoiceLikelihood likelihood)
{
	return m_audioInput.SetActivationLikelihood(likelihood);
}

void MumbleClient::SetInputDevice(const std::string& dsoundDeviceId)
{
	return m_audioInput.SetAudioDevice(dsoundDeviceId);
}

void MumbleClient::SetOutputDevice(const std::string& dsoundDeviceId)
{
	return m_audioOutput.SetAudioDevice(dsoundDeviceId);
}

void MumbleClient::SetPTTButtonState(bool pressed)
{
	return m_audioInput.SetPTTButtonState(pressed);
}

void MumbleClient::SetOutputVolume(float volume)
{
	return m_audioOutput.SetVolume(volume);
}

void MumbleClient::UpdateVoiceTarget(int idx, const VoiceTargetConfig& config)
{
	std::lock_guard<std::recursive_mutex> l(m_clientMutex);
	m_pendingVoiceTargetUpdates[idx] = config;
}

void MumbleClient::SetVoiceTarget(int idx)
{
	m_voiceTarget = idx;
}

std::shared_ptr<lab::AudioContext> MumbleClient::GetAudioContext(const std::string& name)
{
	return m_audioOutput.GetAudioContext(name);
}

void MumbleClient::SetChannel(const std::string& channelName)
{
	if (!m_connectionInfo.isConnected)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> l(m_clientMutex);

	if (channelName == m_curManualChannel)
	{
		return;
	}

	m_curManualChannel = channelName;
}

void MumbleClient::AddListenChannel(const std::string& channelName)
{
	std::lock_guard<std::recursive_mutex> l(m_clientMutex);
	m_curChannelListens.insert(channelName);
}

void MumbleClient::RemoveListenChannel(const std::string& channelName)
{
	std::lock_guard<std::recursive_mutex> l(m_clientMutex);
	m_curChannelListens.erase(channelName);
}

void MumbleClient::SetAudioDistance(float distance)
{
	m_audioInput.SetDistance(distance);
	m_audioOutput.SetDistance(distance);
}

float MumbleClient::GetAudioDistance()
{
	return m_audioOutput.GetDistance();
}

void MumbleClient::SetPositionHook(const TPositionHook& hook)
{
	m_positionHook = hook;
}

float MumbleClient::GetInputAudioLevel()
{
	return m_audioInput.GetAudioLevel();
}

void MumbleClient::SetClientVolumeOverride(const std::wstring& clientName, float volume)
{
	m_state.ForAllUsers([this, &clientName, volume](const std::shared_ptr<MumbleUser>& user)
	{
		if (user->GetName() == clientName)
		{
			GetOutput().HandleClientVolumeOverride(*user, volume);
		}
	});
}

void MumbleClient::SetClientVolumeOverrideByServerId(uint32_t serverId, float volume)
{
	m_state.ForAllUsers([this, serverId, volume](const std::shared_ptr<MumbleUser>& user)
	{
		if (user->GetServerId() == serverId)
		{
			GetOutput().HandleClientVolumeOverride(*user, volume);
		}
	});
}

std::wstring MumbleClient::GetPlayerNameFromServerId(uint32_t serverId)
{
	std::wstring retName;

	m_state.ForAllUsers([serverId, &retName](const std::shared_ptr<MumbleUser>& user)
	{
		if (!retName.empty())
		{
			return;
		}

		if (user && user->GetServerId() == serverId)
		{
			retName = user->GetName();
		}
	});

	return retName;
}

uint32_t MumbleClient::GetVoiceChannelFromServerId(uint32_t serverId)
{
	uint32_t retId = -1;

	m_state.ForAllUsers([serverId, &retId](const std::shared_ptr<MumbleUser>& user)
	{
		if (retId != -1)
		{
			return;
		}

		if (user && user->GetServerId() == serverId)
		{
			retId = user->GetChannelId();
		}
	});

	return retId;
}

void MumbleClient::GetTalkers(std::vector<std::string>* referenceIds)
{
	referenceIds->clear();

	std::vector<uint32_t> sessions;
	m_audioOutput.GetTalkers(&sessions);

	for (uint32_t session : sessions)
	{
		auto user = m_state.GetUser(session);

		if (user)
		{
			referenceIds->push_back(ToNarrow(user->GetName()));
		}
	}

	// local talker talking?
	if (m_audioInput.IsTalking())
	{
		referenceIds->push_back(ToNarrow(m_state.GetUsername()));
	}
}

bool MumbleClient::IsAnyoneTalking()
{
	std::vector<uint32_t> talkers;
	m_audioOutput.GetTalkers(&talkers);

	return (!talkers.empty());
}

void MumbleClient::SetActorPosition(float position[3])
{
	m_audioInput.SetPosition(position);
}

void MumbleClient::SetListenerMatrix(float position[3], float front[3], float up[3])
{
	m_audioOutput.SetMatrix(position, front, up);
}

void MumbleClient::SendVoice(const char* buf, size_t size)
{
	using namespace std::chrono_literals;

	if ((msec() - m_lastUdp) > 7500ms)
	{
		Send(MumbleMessageType::UDPTunnel, buf, size);
	}
	
	if (m_lastUdp != 0ms)
	{
		SendUDP(buf, size);
	}
}

void MumbleClient::SendUDP(const char* buf, size_t size)
{
	if (!m_crypto.GetRef())
	{
		return;
	}

	if (size > 8000)
	{
		return;
	}

	auto outBuf = std::make_shared<std::unique_ptr<char[]>>(new char[8192]);
	m_crypto->Encrypt((const uint8_t*)buf, (uint8_t*)outBuf->get(), size);

	m_loop->EnqueueCallback([this, outBuf, size]()
	{
		m_udp->send(*m_connectionInfo.address.GetSocketAddress(), std::move(*outBuf), size + 4);
	});
}

void MumbleClient::HandleUDP(const uint8_t* buf, size_t size)
{
	if (!m_crypto.GetRef())
	{
		return;
	}

	if (size > 8000)
	{
		return;
	}

	uint8_t outBuf[8192];
	if (!m_crypto->Decrypt(buf, outBuf, size))
	{
		return;
	}

	// update UDP timestamp
	m_lastUdp = msec();

	// handle voice packet
	HandleVoice(outBuf, size - 4);
}

void MumbleClient::HandleVoice(const uint8_t* data, size_t size)
{
	PacketDataStream pds(reinterpret_cast<const char*>(data), size);

	uint8_t header;
	uint64_t sessionId;
	uint64_t sequenceNumber;

	header = pds.next8();

	// ping
	if ((header >> 5) == 1)
	{
		uint64_t timestamp;
		pds >> timestamp;

		// time delta
		auto timeDelta = msec().count() - timestamp;

		// #TODOMUMBLE: unify with TCP pings

		// which ping this is in the history list
		size_t thisPing = m_udpPingCount - 1;

		// move pings down
		if (thisPing >= _countof(m_udpPings))
		{
			for (size_t i = 1; i < _countof(m_udpPings); i++)
			{
				m_udpPings[i - 1] = m_udpPings[i];
			}

			thisPing = _countof(m_udpPings) - 1;
		}

		// store this ping
		m_udpPings[thisPing] = timeDelta;

		// calculate average
		uint32_t avgCount = 0;

		for (size_t i = 0; i < thisPing; i++)
		{
			avgCount += m_udpPings[i];
		}

		m_udpPingAverage = avgCount / float(thisPing + 1);

		// calculate variance
		float varianceCount = 0;

		for (size_t i = 0; i < thisPing; i++)
		{
			auto var = float(m_udpPings[i]) - m_udpPingAverage;
			varianceCount += (var * var);
		}

		m_udpPingVariance = varianceCount / (thisPing + 1);

		return;
	}

	pds >> sessionId;
	pds >> sequenceNumber;

	if ((header >> 5) != 4)
	{
		return;
	}

	auto user = this->GetState().GetUser(uint32_t(sessionId));

	if (!user)
	{
		return;
	}

	uint64_t packetLength = 0;

	do
	{
		pds >> packetLength;

		size_t len = (packetLength & 0x1FFF);
		std::vector<uint8_t> bytes(len);

		if (len > pds.left())
		{
			break;
		}

		for (size_t i = 0; i < len; i++)
		{
			if (pds.left() == 0)
			{
				return;
			}

			uint8_t b = pds.next8();
			bytes[i] = b;
		}

		if (bytes.empty())
		{
			break;
		}

		this->GetOutput().HandleClientVoiceData(*user, sequenceNumber, bytes.data(), bytes.size());

		break;
	} while ((packetLength & 0x2000) == 0);

	if (pds.left() >= 12)
	{
		std::array<float, 3> pos;
		pds >> pos[0];
		pds >> pos[1];
		pds >> pos[2];

		m_positionUpdates.push({ sessionId, pos });

		if (pds.left() >= 4)
		{
			float distance;
			pds >> distance;

			this->GetOutput().HandleClientDistance(*user, distance);
		}
	}
}

void MumbleClient::RunFrame()
{
	decltype(m_positionUpdates)::value_type update;

	while (m_positionUpdates.try_pop(update))
	{
		auto [sessionId, pos] = update;

		auto user = this->GetState().GetUser(uint32_t(sessionId));

		if (user)
		{
			if (m_positionHook)
			{
				auto newPos = m_positionHook(ToNarrow(user->GetName()));

				if (newPos)
				{
					pos = *newPos;
				}
			}

			this->GetOutput().HandleClientPosition(*user, pos.data());
		}
	}
}

void MumbleClient::MarkConnected()
{
	m_completionEvent.set(&m_connectionInfo);
}

MumbleConnectionInfo* MumbleClient::GetConnectionInfo()
{
	return &m_connectionInfo;
}

void MumbleClient::HandlePing(const MumbleProto::Ping& ping)
{
	m_tcpPingCount++;

	if (ping.has_timestamp())
	{
		// time delta
		auto timeDelta = msec().count() - ping.timestamp();

		// which ping this is in the history list
		size_t thisPing = m_tcpPingCount - 1;

		// move pings down
		if (thisPing >= _countof(m_tcpPings))
		{
			for (size_t i = 1; i < _countof(m_tcpPings); i++)
			{
				m_tcpPings[i - 1] = m_tcpPings[i];
			}

			thisPing = _countof(m_tcpPings) - 1;
		}

		// store this ping
		m_tcpPings[thisPing] = timeDelta;

		// calculate average
		uint32_t avgCount = 0;

		for (size_t i = 0; i < thisPing; i++)
		{
			avgCount += m_tcpPings[i];
		}

		m_tcpPingAverage = avgCount / float(thisPing + 1);

		// calculate variance
		float varianceCount = 0;

		for (size_t i = 0; i < thisPing; i++)
		{
			auto var = float(m_tcpPings[i]) - m_tcpPingAverage;
			varianceCount += (var * var);
		}

		m_tcpPingVariance = varianceCount / (thisPing + 1);
	}
}

void MumbleClient::Send(MumbleMessageType type, const char* buf, size_t size)
{
	MumblePacketHeader header;
	header.SetPacketType((uint16_t)type);
	header.SetPacketLength(size);

	Send((const char*)&header, sizeof(header));
	Send(buf, size);
}

void MumbleClient::Send(const char* buf, size_t size)
{
	if (!m_connectionInfo.isConnected)
	{
		return;
	}

	std::unique_lock<std::recursive_mutex> lock(m_clientMutex);

	if (m_tlsClient->is_active())
	{
		m_tlsClient->send((const uint8_t*)buf, size);
	}
}

void MumbleClient::WriteToSocket(const uint8_t buf[], size_t length)
{
	auto outBuf = std::make_shared<std::unique_ptr<char[]>>(new char[length]);
	memcpy(outBuf->get(), buf, length);

	m_loop->EnqueueCallback([this, outBuf, length]()
	{
		m_tcp->write(std::move(*outBuf), length);
	});
}

void MumbleClient::OnAlert(Botan::TLS::Alert alert, const uint8_t[], size_t)
{
	console::DPrintf("Mumble", "TLS alert: %s\n", alert.type_string().c_str());

	if (alert.is_fatal() || alert.type() == Botan::TLS::Alert::CLOSE_NOTIFY)
	{
		m_connectionInfo.isConnecting = false;
		m_connectionInfo.isConnected = false;

		m_connectTimer->start(2500ms, 0s);
	}
}

void MumbleClient::OnReceive(const uint8_t buf[], size_t length)
{
	g_currentMumbleClient = this;

	m_handler.HandleIncomingData(buf, length);
}

bool MumbleClient::OnHandshake(const Botan::TLS::Session& session)
{
	console::DPrintf("Mumble", "Got session %s %s\n", session.version().to_string().c_str(), session.ciphersuite().to_string().c_str());

	return true;
}

void MumbleClient::OnActivated()
{
	// initialize idle timer only *now* that the session is active
	// (otherwise, if the idle timer ran after 500ms from connecting, but TLS connection wasn't set up within those 500ms,
	// the idle event would immediately try to reconnect)
	m_idleTimer->start(500ms, 500ms);

	// send our own version
	MumbleProto::Version ourVersion;
	ourVersion.set_version(0x00010204);
	ourVersion.set_os("Windows");
	ourVersion.set_os_version("Cfx/Embedded");
	ourVersion.set_release("CitizenFX Client");

	this->Send(MumbleMessageType::Version, ourVersion);
}

fwRefContainer<MumbleClient> MumbleClient::GetCurrent()
{
	return g_currentMumbleClient;
}

fwRefContainer<IMumbleClient> CreateMumbleClient()
{
	return new MumbleClient();
}
