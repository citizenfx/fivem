#pragma once

#include <algorithm>
#include <chrono>

// keeps a running deadline so timer wakeup latency doesn't add up to a lower frame rate than requested, and only spins
// for as long as the timer usually wakes up late
class FrameRateLimiter
{
	using Clock = std::chrono::steady_clock;

public:
	FrameRateLimiter()
	{
		using namespace std::chrono_literals;

		m_timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
		m_spinTime = 1ms;

		if (!m_timer)
		{
			m_timer = CreateWaitableTimerW(NULL, TRUE, NULL);
			m_spinTime = 2ms;
		}
	}

	~FrameRateLimiter()
	{
		if (m_timer)
		{
			CloseHandle(m_timer);
		}
	}

	FrameRateLimiter(const FrameRateLimiter&) = delete;
	FrameRateLimiter& operator=(const FrameRateLimiter&) = delete;

	// 0 disables the limit
	void Wait(int fpsLimit)
	{
		using namespace std::chrono_literals;

		if (fpsLimit <= 0)
		{
			return;
		}

		const auto frameTime = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / fpsLimit));
		const auto now = Clock::now();

		m_nextFrame = std::min(m_nextFrame, now + frameTime);

		if (now >= m_nextFrame)
		{
			m_nextFrame = now + frameTime;
			return;
		}

		const auto sleepTime = std::chrono::duration_cast<std::chrono::microseconds>(m_nextFrame - now - m_spinTime);

		if (m_timer && sleepTime > 0us)
		{
			LARGE_INTEGER dueTime;
			dueTime.QuadPart = -(LONGLONG)(sleepTime.count() * 10);

			if (SetWaitableTimer(m_timer, &dueTime, 0, NULL, NULL, FALSE))
			{
				WaitForSingleObject(m_timer, INFINITE);

				UpdateSpinTime(Clock::now() - (now + sleepTime));
			}
		}

		while (Clock::now() < m_nextFrame)
		{
			YieldProcessor();
		}

		m_nextFrame += frameTime;
	}

private:
	void UpdateSpinTime(Clock::duration wakeupLatency)
	{
		using namespace std::chrono_literals;

		const auto target = std::clamp<Clock::duration>(wakeupLatency + 100us, 100us, 4ms);

		if (target > m_spinTime)
		{
			m_spinTime = target;
		}
		else
		{
			m_spinTime -= (m_spinTime - target) / 16;
		}
	}

private:
	HANDLE m_timer = NULL;
	Clock::duration m_spinTime;
	Clock::time_point m_nextFrame;
};
