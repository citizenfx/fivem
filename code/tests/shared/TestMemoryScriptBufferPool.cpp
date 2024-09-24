#include <type_traits>
#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <fxScripting.h>
#include <fxScriptBuffer.h>

#include "TestUtils.h"

TEST_CASE("MemoryScriptBufferPool tests")
{
	SECTION("Constructor test")
	{
		fx::MemoryScriptBuffer::MemoryScriptBufferPool pool(1024);
		REQUIRE(pool.GetResultBuffer(0) == nullptr);
	}

	SECTION("GetResultBuffer test")
	{
		fx::MemoryScriptBuffer::MemoryScriptBufferPool pool(1024);
		char* buffer = pool.GetResultBuffer(512);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		REQUIRE(buffer != nullptr);
		pool.FreeResultBuffer(buffer, 512);
		REQUIRE(pool.GetFreeBlockCount() == 1);
	}

	SECTION("FreeResultBuffer test")
	{
		fx::MemoryScriptBuffer::MemoryScriptBufferPool pool(1024);
		char* buffer = pool.GetResultBuffer(512);
		pool.FreeResultBuffer(buffer, 512);
		char* buffer2 = pool.GetResultBuffer(512);
		REQUIRE(buffer2 != nullptr);
		REQUIRE(buffer == buffer2);
	}

	SECTION("Pool exhaustion test")
	{
		fx::MemoryScriptBuffer::MemoryScriptBufferPool pool(1024);
		char* buffer = pool.GetResultBuffer(1025);
		REQUIRE(buffer == nullptr);
	}

	SECTION("Pool to small fragmentation test")
	{
		fx::MemoryScriptBuffer::MemoryScriptBufferPool pool(1024);
		char* buffer1 = pool.GetResultBuffer(256);
		char* buffer2 = pool.GetResultBuffer(256);
		char* buffer3 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		// Free the middle buffer
		pool.FreeResultBuffer(buffer2, 256);
		REQUIRE(pool.GetFreeBlockCount() == 2);
		char* buffer4 = pool.GetResultBuffer(512);
		REQUIRE(buffer4 == nullptr);
	}

	SECTION("Pool fragmentation test")
	{
		fx::MemoryScriptBuffer::MemoryScriptBufferPool pool(1280);
		char* buffer1 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		char* buffer2 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		char* buffer3 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		char* buffer4 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		// Free the middle buffers
		pool.FreeResultBuffer(buffer2, 256);
		REQUIRE(pool.GetFreeBlockCount() == 2);
		pool.FreeResultBuffer(buffer3, 256);
		REQUIRE(pool.GetFreeBlockCount() == 2);
		char* buffer5 = pool.GetResultBuffer(512);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		REQUIRE(buffer5 != nullptr);
	}

	SECTION("Pool empty fragmentation test")
	{
		fx::MemoryScriptBuffer::MemoryScriptBufferPool pool(1024);
		char* buffer1 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		char* buffer2 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		char* buffer3 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		char* buffer4 = pool.GetResultBuffer(256);
		REQUIRE(pool.GetFreeBlockCount() == 0);
		pool.FreeResultBuffer(buffer1, 256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		pool.FreeResultBuffer(buffer2, 256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		pool.FreeResultBuffer(buffer3, 256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
		pool.FreeResultBuffer(buffer4, 256);
		REQUIRE(pool.GetFreeBlockCount() == 1);
	}
}
