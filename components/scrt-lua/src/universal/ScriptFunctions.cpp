#include "StdInc.h"
#include "scrEngine.h"
#include "CPlayerInfo.h"

static InitFunction initFunction([] ()
{
	using namespace rage;

	scrEngine::RegisterNativeHandler("GET_PLAYER_SERVER_ID", [] (rage::scrNativeCallContext* context)
	{
		int playerIdx = context->GetArgument<int>(0);
		CPlayerInfo* info = CPlayerInfo::GetPlayer(playerIdx);

		if (!info)
		{
			context->SetResult(0, -1);
		}
		else
		{
			context->SetResult(0, info->address.inaOnline.s_addr);
		}
	});
});