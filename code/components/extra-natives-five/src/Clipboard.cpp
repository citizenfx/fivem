#include "StdInc.h"

#include <ScriptEngine.h>

#include "ICoreGameInit.h"
#include "ScriptWarnings.h"

static InitFunction initFunction([]()
{	
	fx::ScriptEngine::RegisterNativeHandler("COPY_TEXT_TO_CLIPBOARD", [](fx::ScriptContext& context)
	{
		if (!Instance<ICoreGameInit>::Get()->HasClipboardPermission)
		{
			fx::scripting::Warningf("natives", "No Permission for clipboard");
			return;
		}
		
		std::string text = context.GetArgument<const char*>(0);
		
		if (text.empty())
		{
			return;
		}
		
		const size_t textSize = text.size() + 1;
		
		if (!OpenClipboard(nullptr))
		{
			return;
		}
		
		EmptyClipboard();
		
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, textSize);
		if (hMem)
		{
			char* ptr = static_cast<char*>(GlobalLock(hMem));

			if (ptr)
			{
				memcpy(ptr, text.c_str(), textSize);

				GlobalUnlock(hMem);
				SetClipboardData(CF_TEXT, hMem);
			}
			else
			{
				GlobalFree(hMem);
			}
		}
		CloseClipboard();
	});
});
