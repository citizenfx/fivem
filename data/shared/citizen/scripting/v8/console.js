// Global console

(function (global) {
    const monitorMode = (GetConvar('monitormode', 'false') == 'true') && IsDuplicityVersion();
    const percent = '%'.charCodeAt(0);

    const stringSpecifier = 's'.charCodeAt(0);
    const decimalSpecifier = 'd'.charCodeAt(0);
    const floatSpecifier = 'f'.charCodeAt(0);
    const oSmallSpecifier = 'o'.charCodeAt(0);
    const oBigSpecifier = 'O'.charCodeAt(0);

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
                    `${format(options.actual).slice(0, 128)} ` +
                    `${options.operator} ${format(options.expected).slice(0, 128)}`
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
            const keyFormatted = format(key);
            const valueFormatted = format(value);

            lines[lines.length] = indentNewLines(`${keyFormatted} => ${valueFormatted}`, '  ');
        }

        return lines.concat('}').join('\n');
    }

    function formatSet(set) {
        const lines = ['Set {'];

        for (const value of set) {
            lines[lines.length] = indentNewLines(format(value), '  ');
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

    function formatValue(arg) {
        switch (true) {
            case arg === null:
                return 'null';
                
            case arg === undefined:
                return 'undefined'

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
                let cache = [];
                let out = JSON.stringify(arg, (key, value) => {
                    if (typeof value === 'object' && value !== null) {
                        if (cache.indexOf(value) !== -1) return;
                        cache.push(value);
                    }
                    return value;
                }, 2);
                cache = null;
                return out;

            case arg.toString() === "[object Error]" || arg instanceof Error:
                return arg.stack || Error.prototype.toString.call(arg);

            default:
                return arg.toString();
        }
    }

    function formatSpecifier(specifier, value) {
        switch (specifier) {
            case stringSpecifier:
                return '' + value;

            case decimalSpecifier:
                return parseInt(value, 10).toString();

            case floatSpecifier:
                return parseFloat(value).toString();

            case oSmallSpecifier:
            case oBigSpecifier:
                return formatValue(value);

            default:
                return false;
        }
    }

    /**
     * WHATWG Spec compliant formatter
     * 
     * @see https://console.spec.whatwg.org/#formatter
     * @param {*} message 
     * @param {*} args 
     */
    function format(message = undefined, ...args) {
        if (typeof message === "undefined") {
            return '';
        }

        const totalArgs = args.length;

        if (totalArgs === 0) {
            return formatValue(message);
        }

        const result = [];
        let usedArgs = 0;

        if (typeof message !== "string") {
            result[result.length] = formatValue(message);
        } else {
            const messageLastIndex = message.length - 1;
            let formattedMessage = '';
            let i = 0;

            for (; i < messageLastIndex; i++) {
                const char = message.charAt(i);

                if (char.charCodeAt(0) === percent && totalArgs > usedArgs) {
                    const result = formatSpecifier(message.charCodeAt(i + 1), args[usedArgs]);

                    if (result !== false) {
                        formattedMessage += result;

                        i++;
                        usedArgs++;

                        continue;
                    }
                }

                formattedMessage += char;
            }

            if (i === messageLastIndex) {
                formattedMessage += message[messageLastIndex];
            }

            result[result.length] = formattedMessage;
        }        

        return result.concat(args.slice(usedArgs).map(formatValue)).join(' ');
    }

    class Console {
        constructor() {
            this._trace = global['Citizen'].trace || global['print'];

            this._timers = new Map();
            this._counters = new Map();

            // TODO: Improve the sync console output.
            this.log = (monitorMode) ? this.logSync.bind(this) : this.log.bind(this);
            this.info = this.info.bind(this);
            this.warn = this.warn.bind(this);
            this.time = this.time.bind(this);
            this.clear = this.clear.bind(this);
            this.group = this.group.bind(this);
            this.error = this.error.bind(this);
            this.trace = this.trace.bind(this);
            this.table = this.table.bind(this);
            this.dirxml = this.dirxml.bind(this);
            this.select = this.select.bind(this);
            this.assert = this.assert.bind(this);
            this.timeEnd = this.timeEnd.bind(this);
            this.groupEnd = this.groupEnd.bind(this);
            this.groupCollapsed = this.groupCollapsed.bind(this);
            this.msIsIndependentlyComposed = this.msIsIndependentlyComposed.bind(this);
        }

        table() {
            this.error('console.table is not implemented yet');
        }

        dirxml() {
            this.error('console.dirxml is not implemented yet');
        }

        clear() {
            this.error('console.clear is not implemented yet');
        }

        group() {
            this.error('console.group is not implemented yet');
        }

        groupCollapsed() {
            this.error('console.groupCollapsed is not implemented yet');
        }

        groupEnd() {
            this.error('console.groupEnd is not implemented yet');
        }

        msIsIndependentlyComposed() {
            this.error('console.msIsIndependentlyComposed is not implemented yet');
        }

        select() {
            this.error('console.select is not implemented yet');
        }

        log(message = undefined, ...optionalParams) {
            this._trace(format(message, ...optionalParams));
        }

        logSync(message = undefined, ...optionalParams) {
            process.stdout.write(format(message, ...optionalParams) + "\n");
        }

        debug(message = undefined, ...optionalParams) {
            this._trace('Debug: ' + format(message, ...optionalParams));
        }

        info(message = undefined, ...optionalParams) {
            this._trace('Info: ' + format(message, ...optionalParams));
        }

        warn(message = undefined, ...optionalParams) {
            this._trace('^3Warning: ' + format(message, ...optionalParams) + '^7');
        }

        error(message = undefined, ...optionalParams) {
            this._trace('^1Error: ' + format(message, ...optionalParams) + '^7');
        }

        trace(message = undefined, ...optionalParams) {
            const err = {
                name: 'Trace',
                message: format(message, ...optionalParams)
            };

            Error.captureStackTrace(err, this.trace);

            this._trace(err.stack);
        }

        dir(item, options) {
            this._trace('Dir: ', formatValue(item));
        }

        count(label) {
            if (label === undefined) {
                label = 'default'
            }
            const counter = this._counters.get(label) ? this._counters.get(label) + 1 : 1;

            this._counters.set(label, counter);

            this.log(`${label}: ${counter}`);
        }

        countReset(label) {
            if (label === undefined) {
                label = 'default'
            }
            if (this._counters.get(label) === undefined) {
                this.warn(`Counter "${label}" doesn't exist.`);
                return
            }

            this._counters.set(label, undefined);

            this.log(`${label}: 0`);
        }

        time(label) {
            this._timers.set(label, Citizen.getTickCount());
        }

        timeEnd(label) {
            if (!this._timers.has(label)) {
                this.warn(`No such label ${label} for console.timeEnd()`);
                return;
            }

            const duration = Citizen.getTickCount() - this._timers.get(label);

            this.log(`${label}: ${duration} ms`);

            this._timers.delete(label);
        }

        assert(expression, ...args) {
            if (!expression) {
                throw new AssertionError({
                    actual: expression,
                    expected: true,
                    message: format.apply(null, args),
                    operator: '==',
                    stackStartFunction: this.assert
                });
            }
        }
    }

    global.console = new Console();
})(this || globalThis);
