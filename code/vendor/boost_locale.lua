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

		files_project(os.getenv("BOOST_ROOT") .. '/libs/locale/src/')
		{
			'encoding/codepage.cpp',
			'shared/date_time.cpp',
			'shared/format.cpp',
			'shared/formatting.cpp',
			'shared/generator.cpp',
			'shared/ids.cpp',
			'shared/localization_backend.cpp',
			'shared/message.cpp',
			'shared/mo_lambda.cpp',
			'util/codecvt_converter.cpp',
			'util/default_locale.cpp',
			'util/info.cpp',
			'util/locale_data.cpp',
			'win32/collate.cpp',
			'win32/converter.cpp',
			'win32/numeric.cpp',
			'win32/win_backend.cpp',
			'win32/lcid.cpp',
			'util/gregorian.cpp',
		}
	end
}
