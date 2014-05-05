#include "StdInc.h"
#include "fiDevice.h"

#define PURECALL() __asm { jmp _purecall }

namespace rage
{
fiDeviceImplemented::fiDeviceImplemented()
{
}

fiDeviceImplemented::~fiDeviceImplemented()
{

}

uint32_t WRAPPER fiDeviceImplemented::open(const char* fileName, bool) { PURECALL(); }

uint32_t WRAPPER fiDeviceImplemented::openBulk(const char* fileName, uint64_t* ptr)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::createLocal(const char* fileName)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::create(const char*)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::read(uint32_t handle, void* buffer, uint32_t toRead)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::readBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::writeBulk(int, int, int, int, int)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::write(int, void*, int)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::seek(uint32_t handle, int32_t distance, uint32_t method)
{
	PURECALL();
}

void WRAPPER fiDeviceImplemented::close(uint32_t handle)
{
	PURECALL();
}

void WRAPPER fiDeviceImplemented::closeBulk(uint32_t handle)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::fileLength(uint32_t handle)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_34()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_38()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::rename(const char* from, const char* to)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::mkdir(const char* dir)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_44()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_48()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_4C()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_50()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::findFirst(const char* path, fiFindData* findData)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::findNext(int handle, fiFindData* findData)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::findClose(int handle)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_60()
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::getFileAttributes(const char* path)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_68()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_6C()
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::getParentHandle()
{
	PURECALL();
}
}