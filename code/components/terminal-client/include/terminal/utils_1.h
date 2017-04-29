/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

namespace terminal
{
//
// Title utilities interface.
//
class IUtils1
{
public:
	//
	// The public interface identifier for this interface.
	//
	enum : uint64_t { InterfaceID = 0x050bc41a77c7ec7c };

public:
	//
	// Sends an extensible 'random string' message to the remote Terminal server.
	//
	virtual void SendRandomString(const std::string& string) = 0;
};
}