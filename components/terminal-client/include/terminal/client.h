/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

/*
 * Terminal client primary public interface.
 */

namespace terminal
{
class ConnectRemoteDetail;

class IClient : public fwRefCountable
{
public:
	//
	// The public interface identifier for this interface.
	//
	enum : uint64_t { InterfaceID = 0x2370fd942d6d7c10 };

public:
	//
	// Connects the client to a specified platform server.
	//
	virtual concurrency::task<Result<ConnectRemoteDetail>> ConnectRemote(const char* hostname, uint16_t port) = 0;

	//
	// Obtains an instance of a user service compatible with the specified interface identifier.
	//
	virtual void* GetUserService(uint64_t interfaceIdentifier) = 0;
};

class ConnectRemoteDetail
{
private:
	uint64_t m_hostID;

public:
	inline ConnectRemoteDetail()
	{

	}

	inline ConnectRemoteDetail(uint64_t hostID)
		: m_hostID(hostID)
	{

	}

	inline uint64_t GetHostID() const
	{
		return m_hostID;
	}
};
}