// Timers

(function (global) {
    let gameTime = Citizen.getTickCount();

    const timers = {};
    let timerId = 0;
    const tickers = [];
    let animationFrames = [];

    function  setTimer(timer, callback, interval) {
        timers[timer.id] = {
            callback,
            interval,
            lastRun: gameTime
        };
    }

    function nextId() {
        return { id: timerId++, unref() {}, ref() {} };
    }

    function setTick(callback) {
        tickers[tickers.length] = callback;
    }

    function clearTimer(timer) {
        if (!timer) { return; }
    
        delete timers[timer.id];
    }

    function setTick(callback) {
        tickers[tickers.length] = callback;
    }

    function requestAnimationFrame(callback) {
        animationFrames[animationFrames.length] = callback;
    }

    function setInterval(callback, interval) {
        const id = nextId();

        setTimer(id, callback, interval);

        return id;
    }

    function setTimeout(callback, timeout, ...argsForCallback) {
        const id = nextId();

        setTimer(
            id,
            function() {
				try {
					callback(...argsForCallback);
				} finally {
					clearTimer(id);
				}
            },
            timeout
        );

        return id;
    }

    function setImmediate(callback, ...argsForCallback) {
        return setTimeout(callback, 0, ...argsForCallback);
    }

    function onTick() {
        const localGameTime = Citizen.getTickCount(); // ms
        let i;

        for (const id in timers) {
            const timer = timers[id];

            if ((localGameTime - timer.lastRun) > timer.interval) {
                try {
                    timer.callback();
                } catch(e) {
                    console.error('Unhandled error: ' + e.toString() + '\n' + e.stack);
                }

                timer.lastRun = localGameTime;
            }
        }

        // Process tickers
        if (tickers.length > 0) {
            i = tickers.length;

            while (i--) {
                try {
                    tickers[i]();
                } catch(e) {
                    console.error('Unhandled error: ' + e.toString() + '\n' + e.stack);
                }
            }
        }

        // Process animation frames
        if (animationFrames.length !== 0) {
            const currentAnimationFrames = animationFrames;
            animationFrames = [];

            i = currentAnimationFrames.length;

            while (i--) {
                try {
                    currentAnimationFrames[i]();
                } catch(e) {
                    console.error('Unhandled error: ' + e.toString() + '\n' + e.stack);
                }
            }
        }

        gameTime = localGameTime;
				
        //Manually fire the callbacks that were enqueued by process.nextTick.
        //Since we override setImmediate/etc, this doesn't happen automatically.
        if (global.process && typeof global.process._tickCallback === "function")
          global.process._tickCallback();
    }

    global.setTimeout = setTimeout;
	global.clearTimeout = clearTimer;

	global.setInterval = setInterval;
	global.clearInterval = clearTimer;

	global.setImmediate = setImmediate;
	global.clearImmediate = clearTimer;

    global.setTick = setTick;
    global.requestAnimationFrame = requestAnimationFrame;
    
    global.Citizen.setTickFunction(onTick);
})(this || window);
