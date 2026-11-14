/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <functional>
#include <string>

struct ConfirmationModal
{
	struct Snapshot
	{
		uint64_t generation = 0;
		std::string title;
		std::string desc;
	};

	std::atomic<bool> m_isActive{ false };
	std::mutex m_mutex;
	std::string m_title;
	std::string m_desc;
	std::function<void(bool)> m_callback;

	uint64_t m_generation = 0;

	bool IsActive() const
	{
		return m_isActive.load();
	}

	void Set(const std::string& title, const std::string& desc, std::function<void(bool)> callback)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_title = title;
		m_desc = desc;
		m_callback = std::move(callback);
		++m_generation;
		m_isActive.store(true);
	}

	Snapshot GetSnapshot()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return Snapshot{ m_generation, m_title, m_desc };
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		ClearLocked();
	}

	void Finish(uint64_t generation, bool result)
	{
		std::function<void(bool)> callback;

		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (!m_isActive.load() || generation != m_generation)
			{
				return;
			}

			callback = std::move(m_callback);
			ClearLocked();
		}

		if (callback)
		{
			callback(result);
		}
	}

private:
	void ClearLocked()
	{
		m_title.clear();
		m_desc.clear();
		m_callback = nullptr;
		++m_generation;
		m_isActive.store(false);
	}
};

extern ConfirmationModal g_confirmationModal;

void DrawConfirmationModal();
bool IsConfirmationModalVisible();
