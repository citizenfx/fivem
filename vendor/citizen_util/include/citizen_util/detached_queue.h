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

private:
	// alignas to avoid false sharing
	alignas(128) std::atomic<key*> m_head = &m_stub;
	alignas(128) key* m_tail = &m_stub;
	key m_stub{};

	T* pop_checked(member_reference_t<T, key> ref, bool &empty)
	{
		// pop happens from the end, so tail is checked first
		auto* tail = m_tail;
		auto* next = m_tail->next.load(std::memory_order_acquire);
		if (tail == &m_stub)
		{
			if (next == nullptr)
			{
				// the tail is the stub and the next pointer is a nullptr
				// this is the indicator to know that its empty
				empty = true;
				return nullptr;
			}

			m_tail = next;
			tail = next;
			next = tail->next.load(std::memory_order_acquire);
		}

		if (next != nullptr)
		{
			empty = false;
			m_tail = next;
			return tail->get(std::move(ref));
		}
		
		auto* head = m_head.load(std::memory_order_acquire);
		if (tail != head)
		{
			// pop can be retried, the queue is probably not empty
			empty = false;
			return nullptr;
		}

		push(&m_stub);
		next = tail->next.load(std::memory_order_acquire);
		if (next != nullptr)
		{
			empty = false;
			m_tail = next;
			return tail->get(std::move(ref));
		}

		empty = false;
		return nullptr;
	}

public:
	/// <summary>
	/// Pushes a new element to the beginning queue. This operation is thread-safe.
	/// </summary>
	/// <param name="node">The element to push to the queue.</param>
	void push(key* node)
	{
		// the new pushed element should not have a next, because it will become the head
		node->next.store(nullptr, std::memory_order_relaxed);
		// the new element becomes the head
		auto* prev = m_head.exchange(node, std::memory_order_acq_rel);
		// the previous head adds the new head as the next element in the chain
		prev->next.store(node, std::memory_order_release);
	}

	/// <summary>
	/// Pops a element from the end of the queue. This operation is NOT thread-safe.
	/// </summary>
	/// <param name="ref">The element ref to correctly fetch the pointer from the detached_queue_key.</param>
	/// <returns>Returns a pointer to the popped element, the returned pointer might be a nullptr in the middle of a push.</returns>
	T* pop(member_reference_t<T, key> ref)
	{
		bool empty;
		return pop_checked(ref, empty);
	}

	/// <summary>
	/// Pops a element from the end of the queue. Retries until a element got popped or until the operation knows that the queue is really empty. This operation is NOT thread-safe.
	/// </summary>
	/// <param name="ref">The element ref to correctly fetch the pointer from the detached_queue_key.</param>
	/// <returns>Returns a pointer to the popped element, the returned pointer is only a nullptr if the queue is empty.</returns>
	T* pop_until_empty(member_reference_t<T, key> ref)
	{
		bool empty = false;
		while (!empty)
		{
			if (T* entry = pop_checked(ref, empty))
			{
				return entry;
			}
		}
		return nullptr;
	}
};
}
