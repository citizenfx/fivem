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

					residualTime += now;

					lastTime = msec().count();

					if (now >= 150)
					{
						trace("hitch warning: frame time of %d milliseconds\n", now);
					}

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
