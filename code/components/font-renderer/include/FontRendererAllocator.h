/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
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