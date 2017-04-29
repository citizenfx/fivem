#include "StdInc.h"

#include <fiCustomDevice.h>

class ObfuscatedDevice : public rage::fiCustomDevice
{
private:
	rage::fiDevice* m_device;
	std::string m_fileName;

public:
	ObfuscatedDevice(rage::fiDevice* parent, const std::string& fileName)
		: m_device(parent), m_fileName(fileName)
	{
	}

	virtual uint64_t Open(const char* fileName, bool readOnly) override
	{
		return m_device->Open(m_fileName.c_str(), readOnly);
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) override
	{
		return m_device->OpenBulk(m_fileName.c_str(), ptr);
	}

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*) override
	{
		return OpenBulk(fileName, ptr);
	}

	virtual uint64_t Create(const char* fileName) override
	{
		return -1;
	}

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) override
	{
		return m_device->Read(handle, buffer, toRead);
	}

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) override
	{
		return m_device->ReadBulk(handle, ptr, buffer, toRead);
	}

	virtual int m_40(int a) override
	{
		return m_device->m_40(a);
	}

	virtual rage::fiDevice* GetUnkDevice() override
	{
		return m_device->GetUnkDevice();
	}

	virtual void m_xx() override
	{
		return m_device->m_xx();
	}

	virtual int32_t GetCollectionId() override
	{
		return m_device->GetCollectionId();
	}

	virtual bool m_ax() override
	{
		return m_device->m_ax();
	}

	virtual uint32_t Write(uint64_t, void*, int) override
	{
		return -1;
	}

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int) override
	{
		return -1;
	}

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) override
	{
		return m_device->Seek(handle, distance, method);
	}

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) override
	{
		return m_device->SeekLong(handle, distance, method);
	}

	virtual int32_t Close(uint64_t handle) override
	{
		return m_device->Close(handle);
	}

	virtual int32_t CloseBulk(uint64_t handle) override
	{
		return m_device->CloseBulk(handle);
	}

	virtual int GetFileLength(uint64_t handle) override
	{
		return m_device->GetFileLength(handle);
	}

	virtual uint64_t GetFileLengthLong(const char* fileName) override
	{
		return m_device->GetFileLengthLong(m_fileName.c_str());
	}

	virtual uint64_t GetFileLengthUInt64(uint64_t handle) override
	{
		return m_device->GetFileLengthUInt64(handle);
	}

	virtual bool RemoveFile(const char* file) override
	{
		return false;
	}

	virtual int RenameFile(const char* from, const char* to) override
	{
		return false;
	}

	virtual int CreateDirectory(const char* dir) override
	{
		return false;
	}

	virtual int RemoveDirectory(const char* dir) override
	{
		return false;
	}

	virtual uint32_t GetFileTime(const char* file) override
	{
		return m_device->GetFileTime(m_fileName.c_str());
	}

	virtual bool SetFileTime(const char* file, FILETIME fileTime) override
	{
		return false;
	}

	virtual uint32_t GetFileAttributes(const char* path) override
	{
		return m_device->GetFileAttributes(m_fileName.c_str());
	}

	virtual int m_yx() override
	{
		return m_device->m_yx();
	}

	virtual bool IsBulkDevice() override
	{
		return m_device->IsBulkDevice();
	}

	virtual const char* GetName() override
	{
		return "RageVFSDeviceAdapter";
	}

	virtual int GetResourceVersion(const char* fileName, rage::ResourceFlags* version) override
	{
		return m_device->GetResourceVersion(m_fileName.c_str(), version);
	}

	virtual uint64_t CreateLocal(const char* fileName) override
	{
		return m_device->CreateLocal(m_fileName.c_str());
	}

	virtual void* m_xy(void* a, int len, void* c) override
	{
		if (strstr((char*)c, "resource_surrogate:/") != nullptr)
		{
			char* fn = (char*)c;

			std::string f = &fn[20];
			f = f.substr(0, f.find_last_of("."));

			::CreateDirectory(MakeRelativeCitPath(L"cache\\dunno").c_str(), nullptr);

			FILE* file = _wfopen(MakeRelativeCitPath(L"cache\\dunno\\" + ToWide(f) + L".rpf").c_str(), L"w");

			if (file)
			{
				fclose(file);
			}

			auto device = rage::fiDevice::GetDevice(va("rescache:/dunno/%s.rpf", f), true);

			if (device)
			{
				return device->m_xy(a, len, (void*)va("rescache:/dunno/%s.rpf", f));
			}
		}

		return m_device->m_xy(a, len, (void*)m_fileName.c_str());
	}
};

static InitFunction initFunction([]()
{
	rage::fiDevice::OnInitialMount.Connect([]()
	{
		ObfuscatedDevice* obfuscatedDevice = new ObfuscatedDevice(rage::fiDevice::GetDevice("citizen:/streaming_surrogate.rpf", true), "citizen:/streaming_surrogate.rpf");
		rage::fiDevice::MountGlobal("resource_surrogate:/", obfuscatedDevice, true);
	}, 1500);
});