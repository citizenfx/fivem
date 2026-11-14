// Timers

(function(global) {
    let gameTime = Citizen.getTickCount();

    const timers = {};
    let timerId = 0;
    let activeTimersCount = 0;

    const tickers = {};
    let tickerId = 0;
    let activeTickersCount = 0;

    let animationFrames = [];

    // On the server (Node runtime) the resource's uv loop is polled every millisecond, but our
    // tick-based timers only run once per server frame (50 ms). Keep Node's own setImmediate
    // there so libraries that yield with it (e.g. database drivers) don't wait a whole frame.
    const nativeSetImmediate = (global.process && global.process.versions && global.process.versions.node && typeof global.setImmediate === 'function')
        ? global.setImmediate
        : null;
    const nativeClearImmediate = nativeSetImmediate ? global.clearImmediate : null;
    const nativeImmediates = new WeakSet();
    let immediateScopeName = null;

    function getImmediateScopeName() {
        if (immediateScopeName === null) {
            immediateScopeName = `setImmediate (${GetCurrentResourceName()})`;
        }

        return immediateScopeName;
    }

    function normalizeInterval(interval) {
        return Math.max(1, Math.min(2147483647, interval || 1)) | 0;
    }

    function setTimer(timer, callback, interval) {
        activeTimersCount++;

        timers[timer.id] = {
            callback,
            interval: normalizeInterval(interval),
            lastRun: gameTime,
        };
    }

    function nextTimerId() {
        const id = ++timerId;

        return {
            id,
            unref() {
                return this;
            },
            ref() {
                return this;
            },
            hasRef() {
                return true;
            },
            refresh() {
                const timer = timers[id];
                if (timer) {
                    timer.lastRun = gameTime;
                }
                return this;
            },
            [Symbol.toPrimitive]() {
                return id;
            },
        };
    }

    function nextTickerId() {
        return ++tickerId;
    }

    function setTick(callback) {
        if (typeof callback !== 'function') {
            throw new TypeError(`Callback must be a function. Received ${callback}`);
        }

        const id = nextTickerId();

        activeTickersCount++;

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

        if (tickers[ticker]) {
            activeTickersCount--;
            delete tickers[ticker];
        }
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

        if (nativeImmediates.has(timer)) {
            nativeImmediates.delete(timer);
            nativeClearImmediate(timer);
            return;
        }

        if (timers[timer.id]) {
            activeTimersCount--;
            delete timers[timer.id];
        }
    }

    function requestAnimationFrame(callback) {
        animationFrames[animationFrames.length] = callback;
    }

    function setInterval(callback, interval, ...argsForCallback) {
        if (typeof callback !== 'function') {
            throw new TypeError(`Callback must be a function. Received ${callback}`);
        }

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
        if (typeof callback !== 'function') {
            throw new TypeError(`Callback must be a function. Received ${callback}`);
        }

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
        if (typeof callback !== 'function') {
            throw new TypeError(`Callback must be a function. Received ${callback}`);
        }

        if (nativeSetImmediate) {
            const id = ++timerId;
            const immediate = nativeSetImmediate(() => {
                nativeImmediates.delete(immediate);

                // these run outside the resource tick, so give them their own profiler scope
                const profiling = ProfilerIsRecording();

                if (profiling) {
                    ProfilerEnterScope(getImmediateScopeName());
                }

                try {
                    global.runWithBoundaryStart(() => {
                        try {
                            callback(...argsForCallback);
                        } catch (e) {
                            printError('timer', e);
                        }
                    });
                } finally {
                    if (profiling) {
                        ProfilerExitScope();
                    }
                }
            });

            // keep the same shape as our own timer handles (id, refresh, converts to a number)
            immediate.id = id;
            immediate.refresh = function() {
                return this;
            };
            immediate[Symbol.toPrimitive] = () => id;

            nativeImmediates.add(immediate);
            return immediate;
        }

        return setTimeout(callback, 0, ...argsForCallback);
    }

    function onTick(localGameTime) {
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

            let i = currentAnimationFrames.length;

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
        if (activeTimersCount === 0 && activeTickersCount === 0 && animationFrames.length === 0) {
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
