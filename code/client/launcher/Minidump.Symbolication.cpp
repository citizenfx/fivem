#include <StdInc.h>
#include <dbghelp.h>
#include <psapi.h>

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <json.hpp>
#include <objbase.h>
#include <sstream>
#include <regex>

#include <boost/algorithm/string.hpp>

#include <cpr/cpr.h>

void ParseSymbolicCrash(nlohmann::json& crash, std::string* signature, std::string* stackTrace)
{
	try
	{
		auto& frames = crash["stacktraces"][0]["frames"];

		std::stringstream st;
		int levels = 0;
		
		for (auto& frame : frames)
		{
			std::string frameSig;
			std::string modName = frame.value("package", "");

			if (!modName.empty())
			{
				if (auto bs = modName.find_last_of('\\'); bs != std::string::npos)
				{
					modName = modName.substr(bs + 1);
				}
			}

			if (modName == "FiveM_GTAProcess.exe")
			{
				modName = "GTA5.exe";
			}
			else if (modName == "RedM_GTAProcess.exe")
			{
				modName = "RDR2.exe";
			}

			std::string lineDetail = "";

			if (frame.find("symbol") != frame.end())
			{
				auto symbol = frame.value("symbol", "");

				frameSig = fmt::sprintf(
					"%s!%s (0x%x)",
					modName,
					symbol,
					strtoull(frame.value("instruction_addr", "0x0").c_str() + 2, NULL, 16) - strtoull(frame.value("sym_addr", "0x0").c_str() + 2, NULL, 16)
				);

				if (frame.find("abs_path") != frame.end())
				{
					auto relPath = frame.value("abs_path", "");
					boost::algorithm::replace_all(relPath, "\\", "/");
					std::string appPath;

					if (auto f = relPath.find("fivem/"); f != std::string::npos)
					{
						appPath = relPath.substr(f + 6);
					}

					auto fn = relPath.substr(relPath.find_last_of('/') + 1);
					
					if (!appPath.empty())
					{
						lineDetail = fmt::sprintf(" (<A HREF=\"https://github.com/citizenfx/fivem/blob/master/%s#L%d\">%s:%d</A>)",
						appPath,
						frame.value("lineno", 0),
						fn,
						frame.value("lineno", 0));
					}
					else
					{
						lineDetail = fmt::sprintf(" (%s:%d)", fn, frame.value("lineno", 0));
					}
				}
			}
			else if (frame.find("package") != frame.end())
			{
				auto ia = strtoull(frame.value("instruction_addr", "0x0").c_str() + 2, NULL, 16);
				auto pkg = frame.value("package", "");

				for (auto& module : crash["modules"])
				{
					if (_stricmp(module.value("code_file", "").c_str(), pkg.c_str()) == 0)
					{
						frameSig = fmt::sprintf("%s+%X", modName, ia - strtoull(module.value("image_addr", "0x0").c_str() + 2, NULL, 16));
						break;
					}
				}
			}
			else
			{
				frameSig = frame.value("instruction_addr", "0x0");
			}

			frameSig = std::regex_replace(frameSig, std::regex{ "<lambda_.*?>" }, "<lambda>");

			st << "  " << frameSig << lineDetail << "\n";
			if (signature->empty())
			{
				*signature = frameSig;
			}

			levels++;

			if (levels >= 7)
			{
				break;
			}
		}

		*stackTrace = st.str();
	}
	catch (std::exception& e)
	{
		
	}
}

nlohmann::json SymbolicateCrash(HANDLE hProcess, HANDLE hThread, PEXCEPTION_RECORD er, PCONTEXT ctx)
{
	auto threads = nlohmann::json::array();
	auto modules = nlohmann::json::array();

	std::vector<HMODULE> moduleHandles(4096);
	DWORD cbNeeded;
	EnumProcessModules(hProcess, &moduleHandles[0], 4096 * sizeof(HMODULE), &cbNeeded);

	SymInitializeW(hProcess, L"", TRUE);

	for (int i = 0; i < cbNeeded / sizeof(HMODULE); i++)
	{
		auto moduleHandle = moduleHandles[i];

		IMAGEHLP_MODULEW64 moduleInfo;
		moduleInfo.SizeOfStruct = sizeof(moduleInfo);

		if (!SymGetModuleInfoW64(hProcess, (DWORD64)moduleHandle, &moduleInfo))
		{
			continue;
		}

		// if we don't have CV data, and this happens to be the game module, pretend to load it from disk so we at least get PDB info
		if (moduleInfo.CVData[0] == L'\0')
		{
			HANDLE hFile = CreateFile(moduleInfo.ImageName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			auto hLoad = SymLoadModuleExW(hProcess, hFile, L"game.exe", L"game.exe", 0x280000000, 0, NULL, 0);

			IMAGEHLP_MODULEW64 moduleInfo2;
			moduleInfo2.SizeOfStruct = sizeof(moduleInfo2);

			if (SymGetModuleInfoW64(hProcess, (DWORD64)hLoad, &moduleInfo2))
			{
				memcpy(&moduleInfo.CVData, &moduleInfo2.CVData, sizeof(moduleInfo.CVData));
				memcpy(&moduleInfo.PdbAge, &moduleInfo2.PdbAge, sizeof(moduleInfo.PdbAge));
				memcpy(&moduleInfo.PdbSig70, &moduleInfo2.PdbSig70, sizeof(moduleInfo.PdbSig70));
			}

			SymUnloadModule64(hProcess, hLoad);
		}

		wchar_t uuid[256];
		StringFromGUID2(moduleInfo.PdbSig70, uuid, std::size(uuid));

		auto bracket = wcsrchr(uuid, L'}');
		wchar_t* uuidS = uuid;

		if (bracket)
		{
			bracket[0] = '\0';
			uuidS++;
		}

		modules.push_back(nlohmann::json::object({
			{ "type", "pe" },
			{ "code_id", fmt::sprintf("%x%x", moduleInfo.TimeDateStamp, moduleInfo.ImageSize) },
			{ "code_file", ToNarrow(moduleInfo.ImageName) },
			{ "debug_id", fmt::sprintf("%s-%x", ToNarrow(uuidS), moduleInfo.PdbAge) },
			{ "debug_file", ToNarrow(moduleInfo.CVData) },
			{ "image_addr", fmt::sprintf("0x%x", moduleInfo.BaseOfImage) },
			{ "image_size", moduleInfo.ImageSize },
		}));
	}

	STACKFRAME64 frame = { 0 };
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrPC.Offset = ctx->Rip;
	frame.AddrStack.Mode = AddrModeFlat;
	frame.AddrStack.Offset = ctx->Rsp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = ctx->Rbp;

	auto frames = nlohmann::json::array();
	auto ctx2 = *ctx;

	while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, hProcess, hThread, &frame, &ctx2, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
	{
		frames.push_back(nlohmann::json::object({ 
			{ "instruction_addr", fmt::sprintf("0x%x", frame.AddrPC.Offset) }
		}));
	}

	threads.push_back(nlohmann::json::object({ 
		{ "registers", 
			nlohmann::json::object({ 
				{ "rip", fmt::sprintf("0x%x", ctx->Rip) },
				{ "rsp", fmt::sprintf("0x%x", ctx->Rsp) },
				{ "rbp", fmt::sprintf("0x%x", ctx->Rbp) },
				{ "rax", fmt::sprintf("0x%x", ctx->Rax) },
				{ "rbx", fmt::sprintf("0x%x", ctx->Rbx) },
				{ "rcx", fmt::sprintf("0x%x", ctx->Rcx) },
				{ "rdx", fmt::sprintf("0x%x", ctx->Rdx) },
				{ "rsi", fmt::sprintf("0x%x", ctx->Rsi) },
				{ "rdi", fmt::sprintf("0x%x", ctx->Rdi) },
				{ "r8", fmt::sprintf("0x%x", ctx->R8) },
				{ "r9", fmt::sprintf("0x%x", ctx->R9) },
				{ "r10", fmt::sprintf("0x%x", ctx->R10) },
				{ "r11", fmt::sprintf("0x%x", ctx->R11) },
				{ "r12", fmt::sprintf("0x%x", ctx->R12) },
				{ "r13", fmt::sprintf("0x%x", ctx->R13) },
				{ "r14", fmt::sprintf("0x%x", ctx->R14) },
				{ "r15", fmt::sprintf("0x%x", ctx->R14) },
			})
		},
		{ "frames", frames }
	}));

	auto symb = nlohmann::json::object({ 
		{ "stacktraces", threads },
		{ "modules", modules }
	});

	auto r = cpr::Post(cpr::Url{ "https://crash-ingress.fivem.net/symbolicate?timeout=5" }, cpr::Body{ symb.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace) },
	cpr::Timeout{ std::chrono::seconds(10) }, cpr::Header{ { "content-type", "application/json"} });

	if (!r.error && r.status_code <= 299)
	{
		try
		{
			auto j = nlohmann::json::parse(r.text);
			if (j.value("status", "") == "completed")
			{
				return j;
			}
		}
		catch (std::exception&)
		{
		
		}
	}

	return {};
}
#endif
