// Timers

(function (global) {
    let gameTime = 0;

    const timers = {};
    let timerId = 0;
    const tickers = [];
    let animationFrames = [];

    function  setTimer(id, callback, interval) {
        timers[id] = {
            callback,
            interval,
            lastRun: gameTime
        };
    }

    function nextId() {
        return timerId++;
    }

    function setTick(callback) {
        tickers[tickers.length] = callback;
    }

    function clearTimer(id) {
        delete timers[id];
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

    function setTimeout(callback, timeout) {
        const id = nextId();

        setTimer(
            id,
            function() {
                callback();
                clearTimer(id);
            },
            timeout
        );

        return id;
    }

    function setImmediate(callback) {
        return setTimeout(callback, 0);
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
                    console.error('Unhandled error', e);
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
                    console.error('Unhandled error', e);
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
                    console.error('Unhandled error', e);
                }
            }
        }

        gameTime = localGameTime;
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
