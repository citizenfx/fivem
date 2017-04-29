'use strict';

(function()
{
	let TimerManager = class
	{
		constructor()
		{
			this.timers = {};
			this.animationFrames = [];
			this.timerID = 1;
		}

		setTimeout(callback, ms)
		{
			var id;

			return id = this.setInterval(() =>
			{
				this.clearInterval(id);	

				callback();
			}, ms);
		}

		setInterval(callback, ms)
		{
			let timerID = this.timerID;

			let timerData = {
				callback: callback,
				interval: ms,
				lastRun: 0
			};

			this.timerID++;

			this.timers[timerID] = timerData;

			return timerID;
		}

		clearTimeout(id)
		{
			this.clearInterval(id);
		}

		clearInterval(id)
		{
			delete this.timers[id];
		}

		requestAnimationFrame(cb)
		{
			this.animationFrames.push(cb);
		}

		tick()
		{
			// process timers
			let currentTime = Citizen.getTickCount();

			for (let timerID in this.timers)
			{
				let timer = this.timers[timerID];

				if (currentTime >= (timer.lastRun + timer.interval))
				{
					try
					{
						timer.callback();
					}
					catch (e)
					{
						// TODO: replace with console.log once implemented
						print(e);
						print(e.stack);
					}

					timer.lastRun = currentTime;
				}
			}

			// process animation frames
			for (let animationFrame of this.animationFrames)
			{
				try
				{
					animationFrame();
				}
				catch (e)
				{
					// TODO: replace with console.log once implemented
					print(e);
					print(e.stack);
				}
			}

			this.animationFrames = [];
		}
	};

	// instantiate the timer manager
	let timerManager = new TimerManager();

	// set global functions
	window.setTimeout = timerManager.setTimeout.bind(timerManager);
	window.clearTimeout = timerManager.clearTimeout.bind(timerManager);

	window.setInterval = timerManager.setInterval.bind(timerManager);
	window.clearInterval = timerManager.clearInterval.bind(timerManager);

	window.requestAnimationFrame = timerManager.requestAnimationFrame.bind(timerManager);

	// set the global tick function
	Citizen.setTickFunction(function()
	{
		timerManager.tick();
	});

	print('end Cfx init');
})();