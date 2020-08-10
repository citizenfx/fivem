// Copyright (c) 2020 Can Boluk and contributors of the VTIL Project   
// Modified by DefCon42.
// All rights reserved.   
//    
// Redistribution and use in source and binary forms, with or without   
// modification, are permitted provided that the following conditions are met: 
//    
// 1. Redistributions of source code must retain the above copyright notice,   
//    this list of conditions and the following disclaimer.   
// 2. Redistributions in binary form must reproduce the above copyright   
//    notice, this list of conditions and the following disclaimer in the   
//    documentation and/or other materials provided with the distribution.   
// 3. Neither the name of VTIL Project nor the names of its contributors
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.   
//    
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
// POSSIBILITY OF SUCH DAMAGE.        
//
#pragma once
#include <atomic>
#include <mutex>

#include "type_helpers.h"

namespace fx
{
// Detached key.
//
template<typename T>
struct detached_queue_key
{
	std::atomic<detached_queue_key*> next = nullptr;

	T* get(member_reference_t<T, detached_queue_key> ref)
	{
		return ptr_at<T>(this, -make_offset(ref));
	}
	const T* get(member_reference_t<T, detached_queue_key> ref) const
	{
		return make_mutable(this)->get(std::move(ref));
	}
};

// Extremely simple detached in-place SCSC queue. Slightly faster than a detached MPSC queue due to not needing as much logic for popping.
//
template<typename T>
struct detached_scsc_queue
{
	// Detached key.
	//
	using key = detached_queue_key<T>;

	key* head = nullptr;
	key* tail = nullptr;

	void push(key* node)
	{
		node->next = nullptr;
		if (tail == nullptr)
			tail = node;

		if (head == nullptr)
			head = node;
		else
		{
			head->next = node;
			head = node;
		}
	}

	key* flush()
	{
		auto old_tail = tail;

		head = nullptr;
		tail = nullptr;

		return old_tail;
	}
};

// Detached in-place MPSC queue for tracking already allocated objects
// in a different order with no allocations.
// Based on (read: copied from) http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
//
template<typename T>
struct detached_mpsc_queue
{
	// Detached key.
	//
	using key = detached_queue_key<T>;

	key stub{};
	std::atomic<key*> head = &stub;
	std::atomic<key*> tail = &stub;

	void push(key* node)
	{
		node->next.store(nullptr, std::memory_order_relaxed);
		auto* prev = head.exchange(node, std::memory_order_acq_rel);
		prev->next.store(node, std::memory_order_release);
	}

	T* pop(member_reference_t<T, key> ref)
	{
		auto* t = tail.load(std::memory_order_relaxed);
		auto* next = t->next.load(std::memory_order_acquire);
		if (t == &stub)
		{
			if (next == nullptr)
				return nullptr;
			tail.store(next, std::memory_order_relaxed);
			t = next;
			next = next->next;
		}
		if (next != nullptr)
		{
			tail.store(next, std::memory_order_relaxed);
			return t->get(std::move(ref));
		}
		auto* h = head.load(std::memory_order_relaxed);
		if (t != h)
		{
			return nullptr;
		}
		push(&stub);
		next = t->next;
		if (next != nullptr)
		{
			tail.store(next, std::memory_order_relaxed);
			return t->get(std::move(ref));
		}
		return nullptr;
	}
};
}
