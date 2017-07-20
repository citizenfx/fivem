// Timers

(function (global) {
    class TimerManager {
        constructor () {
            this._timers = new Map();
            this._timerId = 0;
            this._tickers = [];
            this._animationFrames = [];
        }

        _setTimer(id, callback, interval, lastRun = 0) {
            this._timers.set(id, {
                callback,
                interval,
                lastRun
            });
        }

        _nextId() {
            return ++this._timerId;
        }

        clearTimer(id) {
            this._timers.delete(id);
        }

        setTick(callback) {
            this._tickers[this._tickers.length] = callback;
        }

        requestAnimationFrame(callback) {
            this._animationFrames[this._animationFrames.length] = callback;
        }

        setInterval(callback, interval) {
            const id = this._nextId();

            this._setTimer(id, callback, interval);

            return id;
        }

        setTimeout(callback, timeout) {
            const id = this._nextId();

            this._setTimer(
                id,
                () => {
                    callback();
                    this.clearTimer(id);
                },
                timeout
            );

            return id;
        }

        setImmediate(callback) {
            return this.setTimeout(callback, 0);
        }

        onTick() {
            const gameTime = Citizen.getTickCount(); // ms
            let i;

            // Process tickers
            i = this._tickers.length;

            while (i--) {
                try {
                    this._tickers[i]();
                } catch(e) {
                    console.error('Unhandled error', e);
                }
            }

            // Process animation frames
            if (this._animationFrames.length !== 0) {
                const animationFrames = Array.from(this._animationFrames);
                this._animationFrames = [];

                // Process animation frames
                i = animationFrames.length;

                while (i--) {
                    try {
                        animationFrames[i]();
                    } catch(e) {
                        console.error('Unhandled error', e);
                    }
                }
            }

            // Process timers first
            for (const timer of this._timers.values()) {
                if (timer.lastRun === 0) {
                    timer.lastRun = gameTime;
                }

                const diff = gameTime - timer.lastRun;

                if (diff > timer.interval) {
                    timer.lastRun = gameTime;

                    try {
                        timer.callback();
                    } catch(e) {
                        console.error('Unhandled error', e);
                    }
                }
            }
        }
    }

    const timerManager = new TimerManager();
    const clearTimer = timerManager.clearTimer.bind(timerManager);

    global.setTimeout = timerManager.setTimeout.bind(timerManager);
	global.clearTimeout = clearTimer;

	global.setInterval = timerManager.setInterval.bind(timerManager);
	global.clearInterval = clearTimer;

	global.setImmediate = timerManager.setImmediate.bind(timerManager);
	global.clearImmediate = clearTimer;

    global.setTick = timerManager.setTick.bind(timerManager);
    global.requestAnimationFrame = timerManager.requestAnimationFrame.bind(timerManager);
    
    global.Citizen.setTickFunction(timerManager.onTick.bind(timerManager));
})(this || window);
