--[[
 Copyright 2012-2014 Matthew Endsley
 All rights reserved

 Redistribution and use in source and binary forms, with or without
 modification, are permitted providing that the following conditions 
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
]]

--
-- Create an openssl namespace to isolate the plugin
--
	local module = {}
    module._VERSION = "1.0.2f"

	module.printf = function(msg, ...)
		printf("[openssl] " .. msg, ...)
	end

	local opensslimpl = {}
	module.impl = opensslimpl

	-- Public interface
		--
		-- Copy the public openssl headers to the specified location
		--   creates the directory $target_dir/openssl and populates
		--   it
		--
		module.copy_public_headers = function(cfg)
			opensslimpl.verify_cfg(cfg)

			-- create the target directory
			local final_target = path.join(cfg.include_dir, "openssl") .. "/"
			os.mkdir(final_target)

			local libraries = opensslimpl.generate_libraries(cfg.src_dir)
			local name, desc
			for name, desc in pairs(libraries) do
				if not opensslimpl.library_excluded(cfg, name, "crypto/") then
					if desc.public_headers then
						local header
						for _, header in ipairs(desc.public_headers) do
							local source = cfg.src_dir .. name .. "/" .. header
							if not os.isfile(source) then
								if os.isfile(source .. ".in") then
									source = source .. ".in"
								end
							end
							os.copyfile(source, final_target .. header)
						end
					end
				end
			end

		end

		--
		-- Generate the commands needed for the crypto
		-- project
		--
		module.crypto_project = function(cfg)
			opensslimpl.verify_cfg(cfg)
			includedirs {
				cfg.include_dir,
			}

			opensslimpl.set_defaults()
			opensslimpl.generate_defines(cfg)
			
			local ids = {}

			local libraries = opensslimpl.generate_libraries(cfg.src_dir)
			local libname, desc, filename
			for libname, desc in pairs(libraries) do
				if not opensslimpl.library_excluded(cfg, libname, "crypto/") then
					if string.sub(libname, 0, 6) == "crypto"  or libname == "" then
						for _, filename in ipairs(desc.source) do
							files {
								cfg.src_dir .. libname .. "/" .. filename
							}
						end
						for _, filename in ipairs(desc.private_headers) do
							table.insert(ids,
								path.getdirectory(cfg.src_dir .. libname .. "/" .. filename))
						end
					end
				end
			end
			
			table.sort(ids)
			includedirs(ids)
		end

		--
		-- Generate the commands needed for the ssl project
		--
		module.ssl_project = function(cfg)
			opensslimpl.verify_cfg(cfg)
			includedirs {
				cfg.include_dir,
			}

			opensslimpl.set_defaults()
			opensslimpl.generate_defines(cfg)
			
			local ids = {}

			local libraries = opensslimpl.generate_libraries(cfg.src_dir)
			local libname, desc, filename
			for libname, desc in pairs(libraries) do
				if libname == "ssl" or libname == "crypto"  or libname == "" then
					if libname == "ssl" then
						for _, filename in ipairs(desc.source) do
							files {
								cfg.src_dir .. libname .. "/" .. filename
							}
						end
					end
					for _, filename in ipairs(desc.private_headers) do
						table.insert(ids,
							path.getdirectory(cfg.src_dir .. libname .. "/" .. filename))
					end
				end
			end
			
			table.sort(ids)
			includedirs(ids)
		end

	-- Implementation interface

		--
		-- Helper functions for navigating/processing the
		-- OpenSSL source tree
		--

		-- sanity check the current configuration
		opensslimpl.verify_cfg = function(cfg)
			assert(cfg.src_dir, "OpenSSL configuration does not contain a src_dir field")
			assert(cfg.include_dir, "OpenSSL configuration does not contain an include_dir field")
		end

		-- generate system-level defaults
		opensslimpl.set_defaults = function()
			defines {
				"NO_WINDOWS_BRAINDEATH",
			}
			filter {"system:windows"}
				defines {
					"WIN32_LEAN_AND_MEAN",
					"_CRT_SECURE_NO_DEPRECATE",
					"OPENSSL_SYSNAME_WIN32",
					"OPENSSL_NO_EC_NISTP_64_GCC_128"
				}
			filter {"architecture:x32 or architecture:x64"}
				defines {
					"L_ENDIAN",
				}

			filter {}
		end

		-- generate the OPENSSL_NO_* definitions for a configuration
		opensslimpl.generate_defines = function(openssl_cfg)
			if not openssl_cfg then return end
			if not openssl_cfg.excluded_libs then return end

			local name
			for _, name in ipairs(openssl_cfg.excluded_libs) do
				defines {
					"OPENSSL_NO_" .. string.upper(name),
				}
			end
		end

		-- Parse an openssl makefile and generate a library description
		opensslimpl.parse_library = function(pathToMakefile)
			local lib = {
				public_headers = {},
				private_headers = {},
				source = {},
			}

			-- read makefile, one line at a time
			local f = assert(io.open(pathToMakefile))
			local line = ""
			while true do
				local curr = f:read("*line")
				--line = line .. " " .. f:read("*line")
				if not curr then break end
				if #curr ~= 0 then
					line = line .. curr
					if curr:sub(#curr) == "\\" then
						line = line:sub(0, #line-1)
					else
						-- openssl splits header files into two categories:
						--    private headers (HEADER=...)
						--    public headers  (EXHEADER=...)
						-- source files are denoted by LIBSRC=...
						if string.sub(line, 1, 9) == "EXHEADER=" then
							lib.public_headers = opensslimpl.split_words(line:sub(10))
						elseif string.sub(line, 1, 7) == "HEADER=" then
							lib.private_headers = opensslimpl.split_words(line:sub(8))
						elseif string.sub(line, 1, 7) == "LIBSRC=" then
							lib.source = opensslimpl.split_words(line:sub(8))
						end

						line = ""
					end
				else
					line = ""
				end
			end

			-- handle the case where public headers are included in the private_headers
			local idx, header
			for idx, header in ipairs(lib.private_headers) do
				if header == "$(EXHEADER)" then
					lib.private_headers[idx] = nil
					for _, publicheader in ipairs(lib.public_headers) do
						table.insert(lib.private_headers, publicheader)
					end
					break
				end
			end

			return lib
		end

		-- split a line into a series of whitespace-delimited words
		opensslimpl.split_words = function(line)
			local words = {}
			local word
			for word in line:gmatch("%S+") do
				table.insert(words, word)
			end
			return words
		end

		-- Find and generate a description for all OpenSSL libraries
		opensslimpl.generate_libraries = function(OPENSSL_DIR)
			local libraries = {}

			-- find makefiles in crypto/
			local cryptoPrefix = OPENSSL_DIR .. "crypto/"
			local makefile
			for _, makefile in ipairs(os.matchfiles(cryptoPrefix .. "**/Makefile")) do
				local libraryName = path.getdirectory(makefile)
				if string.sub(libraryName, 1, #cryptoPrefix) == cryptoPrefix then
					libraryName = string.sub(libraryName, #cryptoPrefix + 1)
				end
				libraries["crypto/"..libraryName] = opensslimpl.parse_library(makefile)
			end

			-- describe the core crypto library
			libraries["crypto"] = opensslimpl.parse_library(OPENSSL_DIR .. "crypto/Makefile")

			-- describe the SSL library
			libraries["ssl"] = opensslimpl.parse_library(OPENSSL_DIR .. "ssl/Makefile")

			-- describe the core openssl library
			if os.isfile(OPENSSL_DIR .. "Makefile") then
				libraries[""] = opensslimpl.parse_library(OPENSSL_DIR .. "Makefile")
			elseif os.isfile(OPENSSL_DIR .. "Makefile.org") then
				libraries[""] = opensslimpl.parse_library(OPENSSL_DIR .. "Makefile.org")
			end

			return libraries
		end

		-- Determine if a library is excluded via the openssl_config properties
		opensslimpl.library_excluded = function(cfg, libname, prefix)
			if not cfg then return false end
			if not cfg.excluded_libs then return false end

			local v
			for _, v in ipairs(cfg.excluded_libs) do
				if prefix .. v == libname then
					return true
				end
			end

			return false
		end


	return module