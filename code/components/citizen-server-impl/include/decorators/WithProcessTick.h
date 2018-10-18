#pragma once

namespace fx
{
	namespace ServerDecorators
	{
		template<typename TWait, typename TTick>
		const fwRefContainer<fx::GameServer>& WithProcessTick(const fwRefContainer<fx::GameServer>& server, int fps)
		{
			server->SetRunLoop([=]()
			{
				auto lastTime = msec().count();

				auto waiter = TWait();
				auto ticker = TTick();

				auto frameTime = 1000 / fps;

				uint64_t residualTime = 0;

				while (true)
				{
					auto now = msec().count() - lastTime;

					if (now >= 150)
					{
						trace("hitch warning: frame time of %d milliseconds\n", now);
					}

					// clamp time to 200ms to reduce effects of excessive hitches
					if (now > 200)
					{
						now = 200;
					}

					residualTime += now;

					lastTime = msec().count();

					waiter(server, std::max<int>(0, frameTime - residualTime));

					// intervals
					while (residualTime > frameTime)
					{
						residualTime -= frameTime;

						ticker(server, frameTime);
					}
				}
			});

			return server;
		}
	}
}
