return function()
	local RE3_ROOT = '../vendor/re3/'
	
	add_dependencies { 'vendor:bgfx' }
	
	includedirs { "../vendor/bx/include/compat/msvc" }

	local function addSrcFiles( prefix )
		return RE3_ROOT .. prefix .. "/*cpp", RE3_ROOT .. prefix .. "/*.h", RE3_ROOT .. prefix .. "/*.c"
	end
	
	local olddirs = includedirs
	
	local function includedirs(dirs)
		local d = {}
		
		for _, v in ipairs(dirs) do
			table.insert(d, RE3_ROOT .. v)
		end
		
		olddirs(d)
	end

	files { addSrcFiles("src") }
	files { addSrcFiles("src/animation") }
	files { addSrcFiles("src/audio") }
	files { addSrcFiles("src/audio/oal") }
	files { addSrcFiles("src/control") }
	files { addSrcFiles("src/core") }
	files { addSrcFiles("src/entities") }
	files { addSrcFiles("src/math") }
	files { addSrcFiles("src/modelinfo") }
	files { addSrcFiles("src/objects") }
	files { addSrcFiles("src/peds") }
	files { addSrcFiles("src/render") }
	files { addSrcFiles("src/rw") }
	files { addSrcFiles("src/save") }
	files { addSrcFiles("src/skel") }
	files { addSrcFiles("src/skel/glfw") }
	files { addSrcFiles("src/text") }
	files { addSrcFiles("src/vehicles") }
	files { addSrcFiles("src/weapons") }
	files { addSrcFiles("src/extras") }
	files { addSrcFiles("eax") }

	includedirs { "src" }
	includedirs { "src/animation" }
	includedirs { "src/audio" }
	includedirs { "src/audio/oal" }
	includedirs { "src/control" }
	includedirs { "src/core" }
	includedirs { "src/entities" }
	includedirs { "src/math" }
	includedirs { "src/modelinfo" }
	includedirs { "src/objects" }
	includedirs { "src/peds" }
	includedirs { "src/render" }
	includedirs { "src/rw" }
	includedirs { "src/save/" }
	includedirs { "src/skel/" }
	includedirs { "src/skel/glfw" }
	includedirs { "src/text" }
	includedirs { "src/vehicles" }
	includedirs { "src/weapons" }
	includedirs { "src/extras" }
	includedirs { "eax" }
	
	files { addSrcFiles("src/skel/win") }
	includedirs { "src/skel/win" }

	defines { "LIBRW", "RW_RAGE" }
	files { addSrcFiles("src/fakerw") }
	includedirs { "src/fakerw" }

	characterset ("MBCS")
end