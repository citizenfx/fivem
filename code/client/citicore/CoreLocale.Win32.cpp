#include <StdInc.h>
#include <CfxLocale.h>

extern fxlang::LocalizationInstance* GetLocalizationInstance();

extern "C" DLL_EXPORT fxlang::LocalizationInstance* CoreGetLocalization()
{
	static auto li = GetLocalizationInstance();

	return li;
}
