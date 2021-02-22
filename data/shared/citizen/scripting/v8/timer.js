// Timers

(function(global) {
    let gameTime = Citizen.getTickCount();

    const timers = {};
    let timerId = 0;

    const tickers = {};
    let tickerId = 0;

    let animationFrames = [];

    function setTimer(timer, callback, interval) {
        timers[timer.id] = {
            callback,
            interval,
            lastRun: gameTime
        };
    }

    function nextTimerId() {
        return {
            id: ++timerId,
            unref() {},
            ref() {}
        };
    }

    function nextTickerId() {
        return ++tickerId;
    }

    function setTick(callback) {
        const id = nextTickerId();

        tickers[id] = {
            callback,
            promise: null,
        };

        return id;
    }

    function clearTick(ticker) {
        if (!ticker) {
            return;
        }

        delete tickers[ticker];
    }

    function resolveTicker(ticker) {
        if (!tickers[ticker]) {
            return;
        }

        tickers[ticker].promise = null;
    }

    function printTickerError(e) {
        printError('ticker', e);
    }

    function clearTimer(timer) {
        if (!timer) {
            return;
        }

        delete timers[timer.id];
    }

    function requestAnimationFrame(callback) {
        animationFrames[animationFrames.length] = callback;
    }

    function setInterval(callback, interval, ...argsForCallback) {
        const id = nextTimerId();

        setTimer(
            id,
            function() {
        callback(...argsForCallback);
            },
            interval
        );

        return id;
    }

    function setTimeout(callback, timeout, ...argsForCallback) {
        const id = nextTimerId();

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

    function onTick(localGameTime) {
        let i;

        // Process timers
        for (const timerId in timers) {
            const timer = timers[timerId];

            if ((localGameTime - timer.lastRun) > timer.interval) {
                try {
                    timer.callback();
                } catch (e) {
                    printError('timer', e);
                }

                timer.lastRun = localGameTime;
            }
        }

        // Process tickers
        for (const tickerId in tickers) {
            const ticker = tickers[tickerId];

            // If last call of ticker returned a promise,
            // then we should wait for it
            if (ticker.promise !== null && ticker.promise !== undefined) {
                continue;
            }

            let result;
            try {
                result = ticker.callback();
            } catch (e) {
                printTickerError(e);
                continue;
            }

            // We've got a promise!
            if (result !== undefined && result !== null && typeof result.then === 'function') {
                ticker.promise = result
                    .then(resolveTicker.bind(null, tickerId))
                    .catch(printTickerError);
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
                } catch (e) {
                    printError('animationFrame', e);
                }
            }
        }

        gameTime = localGameTime;

        // Manually fire callbacks that were enqueued by process.nextTick.
        // Since we override setImmediate/etc, this doesn't happen automatically.
        if (global.process && typeof global.process._tickCallback === 'function') {
            global.process._tickCallback();
        }
    }

    const defineGlobals = (globals) => {
        Object.defineProperties(global, Object.keys(globals).reduce((acc, name) => {
            acc[name] = {
                value: globals[name],
                writable: false,
                enumerable: true,
                configurable: false,
            };

            return acc;
        }, {}));
    };

    defineGlobals({
        setTick,
        clearTick,
        setTimeout,
        clearTimeout: clearTimer,
        setInterval,
        clearInterval: clearTimer,
        setImmediate,
        clearImmediate: clearTimer,
        requestAnimationFrame,
    });
    
    global.Citizen.setTickFunction(localGameTime => {
        let hasTimer = false;
        let hasTicker = false;
        
        for (let key in timers) {
            hasTimer = true;
            break;
        }
        
        for (let key in tickers) {
            hasTicker = true;
            break;
        }
    
        if (!hasTicker && !hasTimer && animationFrames.length == 0) {
            gameTime = localGameTime;

            // Manually fire callbacks that were enqueued by process.nextTick.
            // Since we override setImmediate/etc, this doesn't happen automatically.
            if (global.process && typeof global.process._tickCallback === 'function') {
                global.process._tickCallback();
            }
            
            return;
        }
    
        global.runWithBoundaryStart(() => {
            onTick(localGameTime);
        });
    });
})(this || window);
