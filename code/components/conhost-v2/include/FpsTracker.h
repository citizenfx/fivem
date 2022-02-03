#pragma once

class FpsTracker
{
public:
	inline FpsTracker()
	{
		std::uninitialized_fill(&previousTimes[0], &previousTimes[std::size(previousTimes)], 0);
	}

	inline void Tick()
	{
		auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
		auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(t - previous);
		previous = t;

		previousTimes[index % std::size(previousTimes)] = frameTime;
		index++;
	}

	inline bool CanGet() const
	{
		return (index > std::size(previousTimes));
	}

	inline uint64_t Get()
	{
		if ((previous - lastGet) > std::chrono::milliseconds{1000})
		{
			std::chrono::microseconds total{ 0 };

			for (int i = 0; i < std::size(previousTimes); i++)
			{
				total += previousTimes[i];
			}

			if (total.count() == 0)
			{
				total = std::chrono::microseconds{ 1 };
			}

			uint64_t fps = ((uint64_t)1000000 * 1000) * std::size(previousTimes) / total.count();
			fps = (fps + 500) / 1000;

			lastValue = fps;
			lastGet = previous;
		}

		return lastValue;
	}

private:
	uint64_t lastValue = 0;
	std::chrono::high_resolution_clock::duration lastGet{ 0 };

	std::chrono::high_resolution_clock::duration previous{ 0 };
	uint32_t index = 0;
	std::chrono::microseconds previousTimes[45];
};
