#include <StdInc.h>
#include <CoreConsole.h>
#include <console/ConsoleWriter.h>
#include <VFSManager.h>
#include <nutsnbolts.h>

#include <fiDevice.h>

#if defined(IS_RDR3)
#define CONFIG_NAME "redm"
#elif defined(GTA_FIVE)
#define CONFIG_NAME "fivem"
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

	OnGameFrame.Connect([]()
	{
		console::GetDefaultContext()->SaveConfigurationIfNeeded("fxd:/" CONFIG_NAME ".cfg");
	});

	rage::fiDevice::OnInitialMount.Connect([]()
	{
		static ConsoleCommand execCommand("exec", [=](const std::string& path)
		{
			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(path);

			if (!stream.GetRef())
			{
				console::Printf("cmd", "No such config file: %s\n", path.c_str());
				return;
			}

			std::vector<uint8_t> data = stream->ReadToEnd();
			data.push_back('\n'); // add a newline at the end

			auto consoleCtx = console::GetDefaultContext();

			consoleCtx->AddToBuffer(std::string(reinterpret_cast<char*>(&data[0]), data.size()));
			consoleCtx->ExecuteBuffer();
		});

		se::ScopedPrincipal seContext(se::Principal{ "system.console" });
		console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "exec", "fxd:/" CONFIG_NAME ".cfg" });
	}, INT32_MAX);
});
