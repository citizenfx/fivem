#include <StdInc.h>

#ifdef LAUNCHER_PERSONALITY_MAIN
#include <CfxLocale.h>

extern fxlang::LocalizationInstance* GetLocalizationInstance();

extern "C" fxlang::LocalizationInstance* CoreGetLocalization()
{
	static auto li = GetLocalizationInstance();

	return li;
}
#endif
