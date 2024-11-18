#include <StdInc.h>

#if !defined(COMPILING_LAUNCH) || defined(LAUNCHER_PERSONALITY_MAIN)
#include <CfxLocale.h>

#ifndef IS_FXSERVER
#include <boost/locale.hpp>

namespace fxlang
{
class LocalizationInstanceImpl : public LocalizationInstance
{
public:
	LocalizationInstanceImpl()
	{
		InitDefaultLocale();
	}

	virtual std::string TranslateString(std::string_view str) override
	{
		return boost::locale::translate(std::string{ str }).str(m_locale);
	}

	virtual std::wstring TranslateString(std::wstring_view str) override
	{
		return boost::locale::translate(std::wstring{ str }).str(m_locale);
	}

	virtual void SetLocale(const std::string& locale) override
	{
		auto wPath = ToWide(locale);
		RegSetKeyValueW(HKEY_CURRENT_USER, L"SOFTWARE\\VMP", L"Global Locale", REG_SZ, wPath.c_str(), (wPath.size() * 2) + 2);

		LoadLocale(locale);
	}

private:
	void InitDefaultLocale()
	{
		wchar_t regPath[32] = { 0 };
		DWORD size = sizeof(regPath);
		RegGetValueW(HKEY_CURRENT_USER, L"SOFTWARE\\VMP", L"Global Locale", RRF_RT_REG_SZ, NULL, regPath, &size);

		LoadLocale(ToNarrow(regPath));
	}

	void LoadLocale(const std::string& name)
	{
		boost::locale::generator gen;
		gen.add_messages_path(ToNarrow(MakeRelativeCitPath(L"citizen/locales/")));
		gen.add_messages_domain("cfx");

		m_locale = gen(fmt::sprintf("%s.UTF-8", name));
	}

private:
	std::locale m_locale;
};
}

fxlang::LocalizationInstance* GetLocalizationInstance()
{
	return new fxlang::LocalizationInstanceImpl();
}
#endif
#endif
