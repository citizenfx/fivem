#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

DEFINE_HANDLER(Version)
{
	auto client = MumbleClient::GetCurrent();

	// send our own version
	MumbleProto::Version ourVersion;
	ourVersion.set_version(0x00010204);
	ourVersion.set_os("Windows");
	ourVersion.set_release("CitizenFX");

	client->Send(MumbleMessageType::Version, ourVersion);

	// also send our initial registration packet
	auto username = client->GetState().GetUsername();
	auto usernameUtf8 = ConvertToUTF8(username);

	MumbleProto::Authenticate authenticate;
	authenticate.set_opus(true);
	authenticate.set_username(usernameUtf8);

	client->Send(MumbleMessageType::Authenticate, authenticate);

	client->EnableAudioInput();
});