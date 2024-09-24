local a = ...

return {
	include = function()
		
	end,

	run = function()
		language "C++"
		kind 'StaticLib'
		
		if a then
			staticruntime "On"
		end
		
		defines { 'BOOST_LOCALE_NO_STD_BACKEND', 'BOOST_LOCALE_NO_POSIX_BACKEND' }
		
		includedirs {'../vendor/boost-submodules/boost-locale/src/'}

		files_project('../vendor/boost-submodules/boost-locale/src/')
		{
			'boost/locale/encoding/codepage.cpp',
			'boost/locale/shared/date_time.cpp',
			'boost/locale/shared/format.cpp',
			'boost/locale/shared/formatting.cpp',
			'boost/locale/shared/generator.cpp',
			'boost/locale/shared/ids.cpp',
			'boost/locale/shared/localization_backend.cpp',
			'boost/locale/shared/message.cpp',
			'boost/locale/shared/mo_lambda.cpp',
			'boost/locale/util/codecvt_converter.cpp',
			'boost/locale/util/default_locale.cpp',
			'boost/locale/util/encoding.cpp',
			'boost/locale/util/info.cpp',
			'boost/locale/util/locale_data.cpp',
			'boost/locale/win32/collate.cpp',
			'boost/locale/win32/converter.cpp',
			'boost/locale/win32/numeric.cpp',
			'boost/locale/win32/win_backend.cpp',
			'boost/locale/win32/lcid.cpp',
			'boost/locale/util/gregorian.cpp',
		}
	end
}
