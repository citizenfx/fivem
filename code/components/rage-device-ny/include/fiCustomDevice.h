#pragma once

#include <fiDevice.h>

namespace rage
{
	class DEVICE_EXPORT fiCustomDevice : public fiDevice
	{
	private:
		uint8_t m_parentDeviceData[16];

		rage::fiDevice* m_parentDeviceRef;

	protected:
		fiCustomDevice();

	public:
		virtual ~fiCustomDevice();

		virtual uint32_t Open(const char* fileName, bool);

		virtual uint32_t OpenBulk(const char* fileName, uint64_t* ptr);

		virtual uint32_t CreateLocal(const char* fileName);

		virtual uint32_t Create(const char*);

		virtual uint32_t Read(uint32_t handle, void* buffer, uint32_t toRead);

		virtual uint32_t ReadBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead);

		virtual uint32_t WriteBulk(int, int, int, int, int);

		virtual uint32_t Write(uint32_t, void*, int);

		virtual uint32_t Seek(uint32_t handle, int32_t distance, uint32_t method);

		virtual uint64_t SeekLong(uint32_t handle, int64_t distance, uint32_t method);

		virtual int32_t Close(uint32_t handle);

		virtual int32_t CloseBulk(uint32_t handle);

		virtual int GetFileLength(uint32_t handle);

		virtual int m_34();
		virtual bool RemoveFile(const char* file);
		virtual int RenameFile(const char* from, const char* to);
		virtual int CreateDirectory(const char* dir);

		virtual int RemoveDirectory(const char* dir);
		virtual uint64_t GetFileLengthLong(const char* file);
		virtual uint64_t GetFileTime(const char* file);
		virtual bool SetFileTime(const char* file, FILETIME fileTime);

		virtual size_t FindFirst(const char* path, fiFindData* findData);
		virtual bool FindNext(size_t handle, fiFindData* findData);
		virtual int FindClose(size_t handle);
		virtual char* FixRelativeName(char* out, size_t s, const char* in) override;
		virtual bool Truncate(uint32_t handle);

		virtual uint32_t GetFileAttributes(const char* path);
		virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes);
		virtual int32_t GetResourceVersion(const char* fileName, ResourceFlags* version) override;
		virtual uint32_t GetParentHandle();
		virtual int64_t m_7C(int) override;
		virtual void m_80() override;
		virtual int32_t m_84(int) override;
	};
}
