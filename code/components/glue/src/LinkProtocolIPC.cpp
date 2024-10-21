/**
 * Link protocol IPC implementation.
 * 
 * This version is one-way protocol using HostSharedData under the hood (which is using file mapping).
 * Master process only listens to messages and child processes only send messages.
 * 
 * Master process knows when message arrived by waiting for sync event to be signaled.
 * 
 * Both master and sender are relying on using shared mutex handle to synchronize read/write to
 * message related fields of shared memory, such as type, size and message itself.
 * 
 * Implementation only supports the following message types for now:
 *  - Auth payload, for handling external authentication in CfxUI.
 *  - Connect to server request.
 */

#include "StdInc.h"

#include <optional>

#include "Error.h"
#include "HostSharedData.h"
#include "LinkProtocolIPC.h"

namespace cfx::glue
{
enum MessageType : uint8_t
{
	MSG_NONE,
	MSG_AUTH_PAYLOAD,
	MSG_CONNECT_TO,
};

typedef std::tuple<MessageType, const std::string> ConsumedMessage;

// Shared memory struct backing IPC
class LPState
{
	class MutexHolder
	{
		HANDLE m_handle;

	public:
		MutexHolder(HANDLE handle)
			: m_handle(handle)
		{
			if (WaitForSingleObject(handle, INFINITE) != WAIT_OBJECT_0)
			{
				FatalError("Failed to acquire LinkProtocolIPC mutex");
			}
		}

		~MutexHolder()
		{
			ReleaseMutex(m_handle);
		}
	};

	DWORD masterProcessPID;

	HANDLE messageMutex;
	uint8_t messageType;
	size_t messageSize;
	wchar_t message[4096];

public:
	LPState()
	{
		masterProcessPID = 0;

		messageMutex = INVALID_HANDLE_VALUE;
		messageType = MSG_NONE;
		messageSize = 0;

		memset(message, 0, sizeof(message));
	}

	inline void InitializeMaster()
	{
		assert(masterProcessPID == 0);
		assert(messageMutex == INVALID_HANDLE_VALUE);

		// Save master process PID,
		// so that child process can use it when duplicating message mutex handle.
		masterProcessPID = GetCurrentProcessId();

		// Security attributes for master message mutex.
		SECURITY_ATTRIBUTES sa = { 0 };
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);

		// Create message mutex to guard write/read operations
		// to message-related fields.
		// 
		// Master process is always listening for messages,
		// so there's no need to close the handle of it manually.
		messageMutex = CreateMutex(&sa, FALSE, NULL);

		// Create the sync event.
		GetEvent();
	}

	inline bool IsMasterInitialized()
	{
		return masterProcessPID != 0 && messageMutex != INVALID_HANDLE_VALUE;
	}

	inline HANDLE GetMessageMutex()
	{
		static HANDLE localMessageMutex = INVALID_HANDLE_VALUE;

		if (localMessageMutex == INVALID_HANDLE_VALUE)
		{
			assert(IsMasterInitialized());

			if (GetCurrentProcessId() == masterProcessPID)
			{
				// Use original message mutex in the master process
				localMessageMutex = messageMutex;
			}
			else
			{
				// Otherwise - we need to duplicate mutex handle to be able to use it.
				// 
				// There's no need to close mutex handle manually because
				// process that sends a message is short-lived and handles will be closed by Windows.
				//
				// If in the future the above is no longer true -
				// we'd potentially want to keep using this mutex handle within the same process,
				// so no need to manually close it either.
				auto masterProcessHandle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, masterProcessPID);

				if (masterProcessHandle == INVALID_HANDLE_VALUE)
				{
					FatalError("Failed to initialize LinkProtocolIPC, unable to open master process");
				}

				auto success = DuplicateHandle(
					masterProcessHandle,
					messageMutex,
					GetCurrentProcess(),
					&localMessageMutex,
					DUPLICATE_SAME_ACCESS,
					0,
					DUPLICATE_SAME_ACCESS
				);

				CloseHandle(masterProcessHandle);

				if (!success)
				{
					FatalError("Failed to initialize LinkProtocolIPC, unable to duplicate mutex");
				}
			}
		}

		return localMessageMutex;
	}

	constexpr size_t GetMaxMessageSize()
	{
		return sizeof(message) / sizeof(wchar_t);
	}

	inline void SendMessageAndNotify(MessageType type, const std::string& _msg)
	{
		if (!IsMasterInitialized())
		{
			FatalError("Failed to send LinkProtocolIPC message, no master process");
		}

		auto msg = ToWide(_msg);
		auto msgSize = msg.size();

		if (msgSize > GetMaxMessageSize())
		{
			FatalError("Failed to send LinkProtocolIPC message, input message is exceeding size limit of %d", GetMaxMessageSize());
		}

		{
			MutexHolder mtx(GetMessageMutex());

			messageType = type;
			messageSize = msgSize;

			memset(message, 0, sizeof(message));
			wcsncpy(message, msg.c_str(), msgSize);
		}

		NotifyMessageAppeared();
	}

	inline HANDLE GetEvent()
	{
		static HANDLE event = INVALID_HANDLE_VALUE;

		if (event == INVALID_HANDLE_VALUE)
		{
			auto eventName = L"Local_LinkProtocolIPC_" LINK_PROTOCOL;

			event = CreateEventW(NULL, TRUE, FALSE, eventName);

			if (event == INVALID_HANDLE_VALUE)
			{
				FatalError("Failed to open LinkProtocolIPC sync event");
			}
		}

		return event;
	}

	inline void NotifyMessageAppeared()
	{
		SetEvent(GetEvent());
	}

	inline bool HasMessageAppeared()
	{
		auto event = GetEvent();
		bool retval = false;
		
		if (WaitForSingleObject(event, 0) == WAIT_OBJECT_0)
		{
			retval = true;
			ResetEvent(event);
		}

		return retval;
	}

	inline std::optional<ConsumedMessage> ConsumeMessage()
	{
		MutexHolder mtx(GetMessageMutex());

		if (messageType == MSG_NONE)
		{
			return std::nullopt;
		}

		// Copy message and type
		const auto type = (MessageType)messageType;
		const auto msg = ToNarrow(std::wstring_view(message, messageSize));

		// Reset message and type
		messageType = MSG_NONE;
		messageSize = 0;
		memset(message, 0, sizeof(message));

		return ConsumedMessage{ type, msg };
	}

	static HostSharedData<LPState>& Get()
	{
		// HostSharedData ensures different launch modes and products will have different name in the end
		static HostSharedData<LPState> state("LinkProtocolIPC");

		return state;
	}
};

#pragma region Send methods
void LinkProtocolIPC::SendConnectTo(const std::string& message)
{
	LPState::Get()->SendMessageAndNotify(MSG_CONNECT_TO, message);
}

void LinkProtocolIPC::SendAuthPayload(const std::string& message)
{
	LPState::Get()->SendMessageAndNotify(MSG_AUTH_PAYLOAD, message);
}
#pragma endregion Send methods

#pragma region Receiver
void LinkProtocolIPC::Initialize()
{
	auto& state = LPState::Get();

	if (state->IsMasterInitialized())
	{
		FatalError("Failed to initialize LinkProtocolIPC, master is already initialized");
	}

	state->InitializeMaster();
}

void LinkProtocolIPC::ProcessMessages()
{
	auto& state = LPState::Get();

	assert(state->IsMasterInitialized());

	if (!LPState::Get()->HasMessageAppeared())
	{
		return;
	}

	auto msg = state->ConsumeMessage();

	if (!msg)
	{
		return;
	}

	auto& [type, message] = msg.value();

	switch (type)
	{
		case MSG_CONNECT_TO:
		{
			OnConnectTo(message);
			break;
		}
		case MSG_AUTH_PAYLOAD:
		{
			OnAuthPayload(message);
			break;
		}
	}
}
#pragma endregion Receiver
}
