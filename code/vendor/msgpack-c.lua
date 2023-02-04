return {
	include = function()
		includedirs {
			"../vendor/msgpack-c/src/",
			"../vendor/msgpack-c/include/",
			"../vendor/msgpack-cpp/src/",
			"../vendor/msgpack-cpp/include/",
			"deplibs/include/msgpack-c/",
		}
	end,

	run = function()
		targetname "msgpack-c"
		language "C++"
		kind "StaticLib"
		defines { "MSGPACK_DLLEXPORT=" }

		-- preprocess the 'cmake' .h.in files
		local projectRoot = '../vendor/msgpack-c'

		for _, v in pairs({ 'sysdep.h', 'pack_template.h' }) do
			local outName = ('%s/include/msgpack/%s'):format(projectRoot, v)
			local inName = ('%s/cmake/%s.in'):format(projectRoot, v)

			local outStat = os.stat(outName)
			local inStat = os.stat(inName)

			if not outStat or outStat.mtime < inStat.mtime then
				io.writefile(outName,
					io.readfile(inName)
						:gsub('@MSGPACK_ENDIAN_BIG_BYTE@', '0')
						:gsub('@MSGPACK_ENDIAN_LITTLE_BYTE@', '1'))
			end
		end

		files
		{
			"../vendor/msgpack-cpp/include/**.hpp",
			"../vendor/msgpack-c/include/**.h",
			"../vendor/msgpack-c/src/*.c" 
		}
	end
}
