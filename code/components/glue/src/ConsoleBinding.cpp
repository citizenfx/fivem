#include <StdInc.h>
#include <CoreConsole.h>
#include <console/ConsoleWriter.h>
#include <VFSManager.h>
#include <nutsnbolts.h>

#include <CL2LaunchMode.h>

#include <fiDevice.h>

#include <ShlObj.h>

#if defined(IS_RDR3)
#define CONFIG_NAME "redm"
#elif defined(GTA_FIVE)
#define CONFIG_NAME "vmp"
#elif defined (GTA_NY)
#define CONFIG_NAME "citizeniv"
#endif

struct ConsoleWriter : public console::IWriter
{
	virtual uint64_t Create(const std::string& path) override
	{
		auto device = vfs::GetDevice(path);
		m_lastDevice = device;

		return device->Create(path);
	}

	virtual void Write(uint64_t handle, const void* data, size_t count) override
	{
		m_lastDevice->Write(handle, data, count);
	}

	virtual void Close(uint64_t handle) override
	{
		m_lastDevice->Close(handle);
		m_lastDevice = {};
	}

	fwRefContainer<vfs::Device> m_lastDevice;
};

static ConsoleWriter g_writer;

static InitFunction initFunction([]()
{
	SetConsoleWriter(&g_writer);

	static bool safeExec = false;

	OnGameFrame.Connect([]()
	{
		if (safeExec)
		{
			console::GetDefaultContext()->SaveConfigurationIfNeeded(fmt::sprintf("fxd:/%s%s.cfg", CONFIG_NAME, launch::IsSDKGuest() ? "_sdk" : ""));
		}
	});

	auto execRoot = []()
	{
		// hide any console prints from this early execution
		auto& printFilterEvent = *console::CoreGetPrintFilterEvent();
		auto filter = printFilterEvent.Connect([](ConsoleChannel&, const char*)
		{
			return false;
		});

		// actually execute commands
		se::ScopedPrincipal seContext(se::Principal{ "system.console" });
		console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "exec", fmt::sprintf("%s%s%s.cfg", (safeExec) ? "fxd:/" : "", CONFIG_NAME, launch::IsSDKGuest() ? "_sdk" : "") });

		// remove the filter
		printFilterEvent.Disconnect(filter);
	};

	rage::fiDevice::OnInitialMount.Connect([execRoot]()
	{
		safeExec = true;
		execRoot();
	}, INT32_MAX);

	static ConsoleCommand execCommand("exec", [=](const std::string& path)
	{
		std::vector<uint8_t> data;

		if (safeExec)
		{
			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(path);

			if (!stream.GetRef())
			{
				console::Printf("cmd", "No such config file: %s\n", path.c_str());
				return;
			}

			data = stream->ReadToEnd();
		}
		else
		{
			PWSTR appDataPath;
			if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
			{
				return;
			}

			// create the directory if not existent
			std::wstring cfxPath = std::wstring(appDataPath) + L"\\VMP";
			CreateDirectory(cfxPath.c_str(), nullptr);

			std::wstring profilePath = cfxPath + L"\\" + ToWide(path);
			CoTaskMemFree(appDataPath);

			FILE* f = _wfopen(profilePath.c_str(), L"rb");

			if (!f)
			{
				return;
			}

			fseek(f, 0, SEEK_END);

			auto len = _ftelli64(f);

			if (len < 0)
			{
				fclose(f);
				return;
			}

			data.resize(len);

			fseek(f, 0, SEEK_SET);
			fread(data.data(), 1, len, f);
			fclose(f);
		}

		data.push_back('\n'); // add a newline at the end

		auto consoleCtx = console::GetDefaultContext();

		consoleCtx->AddToBuffer(std::string(reinterpret_cast<char*>(&data[0]), data.size()));

		try
		{
			consoleCtx->ExecuteBuffer();
		}
		catch (std::exception& e)
		{
			trace("initial console execution failed: %s\n");
		}
	});

	execRoot();
}, INT32_MIN);
