/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

class FrpUseSequentialAllocator
{
public:
	void* operator new[](size_t size);

	void operator delete[](void* ptr);

	void* operator new(size_t size);

	void operator delete(void* ptr);
};

void FrpSeqAllocatorWaitForSwap();

void FrpSeqAllocatorUnlockSwap();

void FrpSeqAllocatorSwapPage();