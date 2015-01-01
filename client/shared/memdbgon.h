/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#if _DEBUG && defined(MEMDBGOK)
#ifndef COMPILING_CORE
#define CORE_MEM_EXPORT __declspec(dllimport)
#else
#define CORE_MEM_EXPORT __declspec(dllexport)
#endif

bool CORE_MEM_EXPORT CoreSetMemDebugInfo(const char* file, size_t line);

void StoreAlloc(void* ptr, size_t size, const char* newFile, const char* newFunc, size_t newLine);
void RemoveAlloc(void* ptr);

static void* operator new(size_t size)
{
	void* ptr = malloc(size);

	CoreSetMemDebugInfo("", 0);

	return ptr;
}

static void operator delete(void* ptr)
{
	free(ptr);
}

static void* operator new[](size_t size)
{
	void* ptr = malloc(size);

	CoreSetMemDebugInfo("", 0);

	return ptr;
}

static void operator delete[](void* ptr)
{
	free(ptr);
}

#define new (CoreSetMemDebugInfo(__FILE__, __LINE__)) && 0 ? nullptr : new
#endif