#pragma once

#ifndef IS_FXSERVER
namespace fxlang
{
class LocalizationInstance
{
public:
	virtual std::string TranslateString(std::string_view str) = 0;

	virtual std::wstring TranslateString(std::wstring_view str) = 0;

	virtual void SetLocale(const std::string& locale) = 0;
};
}

#if defined(COMPILING_CORE) || defined(COMPILING_LAUNCHER)
extern "C" 
#if defined(COMPILING_CORE)
	CORE_EXPORT
#endif
	fxlang::LocalizationInstance* CoreGetLocalization();
#else
static
#ifdef _MSC_VER
__declspec(noinline)
#endif
fxlang::LocalizationInstance* CoreGetLocalization()
{
	static auto localizationInstance = ([]()
	{
		auto func = (fxlang::LocalizationInstance * (*)()) GetProcAddress(GetModuleHandleW(L"CoreRT.dll"), "CoreGetLocalization");

		return func();
	})();

	return localizationInstance;
}
#endif

namespace fxlang
{
inline std::string gettext(std::string_view str)
{
	return CoreGetLocalization()->TranslateString(str);
}

inline std::wstring gettext(std::wstring_view str)
{
	return CoreGetLocalization()->TranslateString(str);
}
}
#else
namespace fxlang
{
inline std::string gettext(std::string_view str)
{
	return std::string{
		str
	};
}
}
#endif

#ifndef FXL_NO_USING
using fxlang::gettext;
#endif
