// Global console

(function (global) {
    const typedArrays = [
        Uint8Array,
        Uint16Array,
        Uint32Array,
        Uint8ClampedArray,
        Int8Array,
        Int16Array,
        Int32Array,
        Float32Array,
        Float64Array
    ];

    class AssertionError extends Error {
        constructor(options) {
            if (typeof options !== 'object' || options === null) {
                throw new TypeError('ERR_INVALID_ARG_TYPE', 'options', 'object');
            }

            if (options.message) {
                super(options.message);
            } else {
                super(
                    `${Console._formatArgs(options.actual).slice(0, 128)} ` +
                    `${options.operator} ${Console._formatArgs(options.expected).slice(0, 128)}`
                );
            }

            this.generatedMessage = !options.message;
            this.name = 'AssertionError [ERR_ASSERTION]';
            this.code = 'ERR_ASSERTION';
            this.actual = options.actual;
            this.expected = options.expected;
            this.operator = options.operator;

            Error.captureStackTrace(this, options.stackStartFunction);
        }
    }

    function indentNewLines(str, indent) {
        return str.split('\n').map(s => indent + s).join('\n');
    }

    function formatMap(map) {
        const lines = ['Map {'];

        for (const [key, value] of map) {
            const keyFormatted = Console._formatArgs(key);
            const valueFormatted = Console._formatArgs(value);

            lines[lines.length] = indentNewLines(`${keyFormatted} => ${valueFormatted}`, '  ');
        }

        return lines.concat('}').join('\n');
    }

    function formatSet(set) {
        const lines = ['Set {'];

        for (const value of set) {
            lines[lines.length] = indentNewLines(Console._formatArgs(value), '  ');
        }

        return lines.concat('}').join('\n');
    }

    function isTypedArray(array) {
        for (const type of typedArrays) {
            if (array instanceof type) {
                return true;
            }
        }

        return false;
    }

    function formatTypedArray(array) {
        return `${array.constructor.name}[ ${array.join(' ')} ]`;
    }

    class Console {
        constructor() {
            this._trace = global['Citizen'].trace || global['print'];

            this._timers = new Map();

            this.log = this.log.bind(this);
            this.info = this.info.bind(this);
            this.warn = this.warn.bind(this);
            this.time = this.time.bind(this);
            this.error = this.error.bind(this);
            this.trace = this.trace.bind(this);
            this.table = this.table.bind(this);
            this.assert = this.assert.bind(this);
            this.timeEnd = this.timeEnd.bind(this);
        }

        static _formatArgs(args) {
            if (!Array.isArray(args)) {
                args = [args];
            }

            return args.map(arg => {
                switch (true) {
                    case arg instanceof WeakMap:
                        return 'WeakMap {}';

                    case arg instanceof WeakSet:
                        return 'WeakSet {}';

                    case arg instanceof Map:
                        return formatMap(arg);

                    case arg instanceof Set:
                        return formatSet(arg);

                    case isTypedArray(arg):
                        return formatTypedArray(arg);

                    case Array.isArray(arg):
                    case arg.toString() === "[object Object]":
                        return JSON.stringify(arg, null, 2);

                    case arg.toString() === "[object Error]" || arg instanceof Error:
                        return arg.stack || Error.prototype.toString.call(arg);

                    default:
                        return arg.toString();
                }
            }).join(' ');
        }

        log(...args) {
            this._trace(Console._formatArgs(args));
        }

        info(...args) {
            this._trace('Info: ' + Console._formatArgs(args));
        }

        warn(...args) {
            this._trace('Warning: ' + Console._formatArgs(args));
        }

        error(...args) {
            this._trace('Error: ' + Console._formatArgs(args));
        }

        trace(...args) {
            const err = {
                name: 'Trace',
                message: Console._formatArgs(args)
            };

            Error.captureStackTrace(err, trace);

            this._trace(err.stack);
        }

        table() {
            this.warn('console.table is not implemented yet');
        }

        time(label) {
            this._timers.set(label, Citizen.getTickCount());
        }

        timeEnd(label) {
            if (!this._timers.has(label)) {
                this.warn(`No such label ${label} for console.timeEnd()`);
                return;
            }

            const duration = Citizen.getTickCount() - this._timers.get(lable);

            this.log(`${label}: ${duration}ms`);

            this._times.delete(label);
        }

        assert(expression, ...args) {
            if (!expression) {
                throw new AssertionError({
                    actual: expression,
                    expected: true,
                    message: Console._formatArgs(args),
                    operator: '==',
                    stackStartFunction: this.assert
                });
            }
        }
    }

    global.console = new Console();
})(this || window);
