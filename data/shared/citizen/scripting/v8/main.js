// CFX JS runtime
/// <reference path="./natives_server.d.ts" />

const EXT_FUNCREF = 10;
const EXT_LOCALFUNCREF = 11;

(function (global) {
	let boundaryIdx = 1;
	let lastBoundaryStart = null;
	const isDuplicityVersion = IsDuplicityVersion();
	const currentResourceName = GetCurrentResourceName();

	// temp
	global.FormatStackTrace = function (args, argLength) {
		return Citizen.invokeNativeByHash(0, 0xd70c3bca, args, argLength, Citizen.resultAsString());
	}

	function getBoundaryFunc(pushFunc, id) {
		return (func, ...args) => {
			const boundary = id || (boundaryIdx++);
			pushFunc(boundary);

			function wrap(...args) {
				return func(...args);
			}

			Object.defineProperty(wrap, 'name', { writable: true });
			wrap.name = `__cfx_wrap_${boundary}`;

			return wrap.call(boundary, ...args);
		};
	}

	global.runWithBoundaryStart = getBoundaryFunc(boundary => {
		Citizen.submitBoundaryStart(boundary);
		lastBoundaryStart = boundary;
	});
	const runWithBoundaryEnd = getBoundaryFunc(Citizen.submitBoundaryEnd);

	let refIndex = 0;
	const nextRefIdx = () => refIndex++;
	const refFunctionsMap = new Map();

	const codec = msgpack.createCodec({
		uint8array: true,
		preset: false,
		binarraybuffer: true
	});

	const pack = data => msgpack.encode(data, { codec });
	const unpack = data => msgpack.decode(data, { codec });

	// store for use by natives.js
	global.msgpack_pack = pack;
	global.msgpack_unpack = unpack;

	/**
	 * @param {Function} refFunction
	 * @returns {string}
	 */
	Citizen.makeRefFunction = (refFunction) => {
		const ref = nextRefIdx();

		refFunctionsMap.set(ref, {
			callback: refFunction,
			refCount: 0
		});

		return Citizen.canonicalizeRef(ref);
	};

	/**
	 * @param {Function} refFunction
	 * @returns {string|null} the function reference, or null if the refFunction that was passed wasn't a function
	 */
	Citizen.getRefFunction = (refFunction) => {
		if (typeof refFunction !== "function") {
			return null;
		}
		return Citizen.makeRefFunction(refFunction);
	}

	function refFunctionPacker(refFunction) {
		const ref = Citizen.makeRefFunction(refFunction);

		return ref;
	}

	function refFunctionUnpacker(refSerialized) {
		const fnRef = Citizen.makeFunctionReference(refSerialized);
		const invoker = GetInvokingResource();

		return function (...args) {
			return runWithBoundaryEnd(() => {
				let success = false;
				let retvals = null;
				try {
					[success, ...retvals] = unpack(fnRef(pack(args)));
				} catch (e) {
				}

				if (retvals === null) {
					let errorMessage = `Error in nested ref call for ${currentResourceName}. `
					// invoker can be null, we don't want to give an even worse
					// error by erroring here :P
					if (invoker) {
						errorMessage += `${currentResourceName} tried to call a function reference in ${invoker} but the reference wasn't valid. `
						if (GetResourceState(invoker) !== "started") {
							errorMessage += `And ${invoker} isn't started, was the resource restarted mid call?`
						} else {
							errorMessage += `(did ${invoker} restart recently?)`
						}
					}

					throw new Error(errorMessage);
				}

				if (success === false) {
					const err = new Error(retvals[0]);
					err.stack = '';
					throw err;
				}

				switch (retvals.length) {
					case 0:
						return undefined;
					case 1:
						const rv = retvals[0];
						if (rv && rv['__cfx_async_retval']) {
							return new Promise((res, rej) => {
								rv['__cfx_async_retval']((v, e) => {
									if (e != null)
										rej(e);
									else {
										switch(v.length) {
											case 0:
												res(undefined);
											case 1:
												res(v[0]);
											default:
												res(v);
										}
									}
								});
							});
						}
						return rv;
					default:
						return retvals;
				}
			});
		};
	}

	const AsyncFunction = (async () => {}).constructor;
	codec.addExtPacker(EXT_FUNCREF, AsyncFunction, refFunctionPacker);
	codec.addExtPacker(EXT_FUNCREF, Function, refFunctionPacker);
	codec.addExtUnpacker(EXT_FUNCREF, refFunctionUnpacker);
	codec.addExtUnpacker(EXT_LOCALFUNCREF, refFunctionUnpacker);

	/**
	 * Deletes ref function
	 * 
	 * @param {int} ref
	 */
	Citizen.setDeleteRefFunction(function (ref) {
		if (refFunctionsMap.has(ref)) {
			const data = refFunctionsMap.get(ref);

			if (--data.refCount <= 0) {
				refFunctionsMap.delete(ref);
			}
		}
	});

	/**
	 * Invokes ref function
	 * 
	 * @param {int} ref 
	 * @param {UInt8Array} args 
	 */
	Citizen.setCallRefFunction(function (ref, argsSerialized) {
		if (!refFunctionsMap.has(ref)) {
			console.error('Invalid ref call attempt:', ref);

			return pack([false]);
		}

		try {
			return runWithBoundaryStart(() => {
				const rv = refFunctionsMap.get(ref).callback(...unpack(argsSerialized));
				if (rv instanceof Promise) {
					return pack([true, {
						'__cfx_async_retval': (cb) => {
						rv.then(v => {
							if (cb != null)
								cb([v], null);
						}).catch(err => {
							if (cb != null) {
								let msg = '';
								if (err) {
									if (err.message) {
										msg = err.message.toString();
									} else {
										msg = err.toString();
									}
								}
								cb(null, msg);
							}
						});
				}
					}]);
				}
				return pack([true, rv]);
			});
		} catch (e) {
			return pack([false, getError('call ref', e)]);
		}
	});

	/**
	 * Duplicates ref function
	 * 
	 * @param {int} ref
	 */
	Citizen.setDuplicateRefFunction(function (ref) {
		if (refFunctionsMap.has(ref)) {
			const refFunction = refFunctionsMap.get(ref);
			++refFunction.refCount;

			return ref;
		}

		return -1;
	});

	// Events
	const emitter = new EventEmitter2();
	const rawEmitter = new EventEmitter2();
	const netSafeEventNames = new Set();

	// Raw events
	global.addRawEventListener = rawEmitter.on.bind(rawEmitter);
	global.addRawEventHandler = global.addRawEventListener;

	// Raw events configuration
	global.setMaxRawEventListeners = rawEmitter.setMaxListeners.bind(rawEmitter);

	// Client events
	global.addEventListener = (name, callback, netSafe = false) => {
		if (netSafe) {
			netSafeEventNames.add(name);
		}

		RegisterResourceAsEventHandler(name);

		emitter.on(name, callback);
	};
	global.on = global.addEventListener;

	// Event Emitter configuration
	global.setMaxEventListeners = emitter.setMaxListeners.bind(emitter);

	// Net events
	global.addNetEventListener = (name, callback) => global.addEventListener(name, callback, true);
	global.onNet = global.addNetEventListener;

	global.removeEventListener = emitter.off.bind(emitter);

	// Convenience aliases for Lua similarity
	global.AddEventHandler = global.addEventListener;
	global.RegisterNetEvent = (name) => void netSafeEventNames.add(name);
	global.RegisterServerEvent = global.RegisterNetEvent;
	global.RemoveEventHandler = global.removeEventListener;

	// Event triggering
	global.emit = (name, ...args) => {
		const dataSerialized = pack(args);

		runWithBoundaryEnd(() => {
			TriggerEventInternal(name, dataSerialized, dataSerialized.length);
		});
	};

	global.TriggerEvent = global.emit;

	if (isDuplicityVersion) {
		global.emitNet = (name, source, ...args) => {
			const dataSerialized = pack(args);

			TriggerClientEventInternal(name, source, dataSerialized, dataSerialized.length);
		};

		global.TriggerClientEvent = global.emitNet;

		global.TriggerLatentClientEvent = (name, source, bps, ...args) => {
			const dataSerialized = pack(args);

			TriggerLatentClientEventInternal(name, source, dataSerialized, dataSerialized.length, bps);
		};

		global.getPlayerIdentifiers = (player) => {
			const numIds = GetNumPlayerIdentifiers(player);
			let t = [];
			for (let i = 0; i < numIds; i++) {
				t[i] = GetPlayerIdentifier(player, i);
			}
			return t;
		};

		global.getPlayerTokens = (player) => {
			const numIds = GetNumPlayerTokens(player);
			let t = [];
			for (let i = 0; i < numIds; i++) {
				t[i] = GetPlayerToken(player, i);
			}
			return t;
		};

		global.getPlayers = () => {
			const num = GetNumPlayerIndices();
			let t = [];

			for (let i = 0; i < num; i++) {
				t[i] = GetPlayerFromIndex(i);
			}

			return t;
		};
	} else {
		global.SendNUIMessage = (data) => {
			const dataJson = JSON.stringify(data)
			SendNuiMessage(dataJson)
		}

		global.emitNet = (name, ...args) => {
			const dataSerialized = pack(args);

			TriggerServerEventInternal(name, dataSerialized, dataSerialized.length);
		};

		global.TriggerServerEvent = global.emitNet;

		global.TriggerLatentServerEvent = (name, bps, ...args) => {
			const dataSerialized = pack(args);

			TriggerLatentServerEventInternal(name, dataSerialized, dataSerialized.length, bps);
		};
	}

	let currentStackDumpError = null;

	function prepareStackTrace(error, trace) {
		const frames = [];
		let skip = false;

		if (error.bs) {
			skip = true;
		}

		if (!error.be) {
			error.be = lastBoundaryStart;
		}

		for (const frame of trace) {
			const functionName = frame.methodName;

			if (functionName && functionName.startsWith('__cfx_wrap_')) {
				const boundary = functionName.substring('__cfx_wrap_'.length) | 0;

				if (boundary == error.bs) {
					skip = false;
				}

				if (boundary == error.be) {
					break;
				}
			}

			if (skip) {
				continue;
			}

			const fn = frame.file;

			if (fn && !fn.startsWith('citizen:/')) {
				const isConstruct = false;
				const isEval = false;
				const isNative = false;
				const methodName = functionName;
				const type = frame.typeName;

				let frameName = '';

				if (isNative) {
					frameName = 'native';
				} else if (isEval) {
					frameName = `eval at ${frame.getEvalOrigin()}`;
				} else if (isConstruct) {
					frameName = `new ${functionName}`;
				} else if (methodName && functionName && methodName !== functionName) {
					frameName = `${type}${functionName} [as ${methodName}]`;
				} else if (methodName || functionName) {
					frameName = `${type}${functionName ? functionName : methodName}`;
				}

				frames.push({
					file: fn,
					line: frame.lineNumber | 0,
					name: frameName
				});
			}
		}

		return frames;
	}

	class StackDumpError {
		constructor(bs, be) {
			this.bs = bs;
			this.be = be;

			Error.captureStackTrace(this);
		}
	}

	function getError(where, e) {
		const stackBlob = global.msgpack_pack(prepareStackTrace(e, parseStack(e.stack)));
		const fst = global.FormatStackTrace(stackBlob, stackBlob.length);

		if (fst !== null && fst !== undefined) {
			return '^1SCRIPT ERROR in ' + where + ': ' + e.toString() + "^7\n" + fst;
		}

		return '';
	}

	global.printError = function (where, e) {
		console.log(getError(where, e));
	}

	Citizen.setStackTraceFunction(function (bs, be) {
		const sde = new StackDumpError(bs, be);
		const rv = pack(prepareStackTrace(sde, parseStack(sde.stack)));

		return rv;
	});

	let errorQueue = [];

	function processErrorQueue() {
		for (const error of errorQueue) {
			console.log(getError('promise (unhandled rejection)', error.error));
		}

		errorQueue = [];
	}

	Citizen.setUnhandledPromiseRejectionFunction(function (event, promise, value) {
		// unhandled
		// we might get a `1` in which case it actually was handled and we should.. un-print
		if (event === 0) {
			let error = '';

			if (value instanceof Error) {
				error = value;
			} else {
				error = new Error((value || '').toString());
			}

			// grab the stack early so it'll remain valid
			const stack = error.stack;

			errorQueue.push({
				error,
				stack,
				promise
			});

			global.setImmediate(processErrorQueue);
		} else if (event === 1) {
			errorQueue = errorQueue.filter(a => a.promise !== promise);
		}
	});

	/**
	 * @param {string} name
	 * @param {UInt8Array} payloadSerialized
	 * @param {string} source
	 */
	Citizen.setEventFunction(function (name, payloadSerialized, source) {
		runWithBoundaryStart(() => {
			global.source = source;

			if (source.startsWith('net')) {
				if (emitter.listeners(name).length > 0 && !netSafeEventNames.has(name)) {
					console.error(`Event ${name} was not safe for net`);

					global.source = null;
					return;
				}

				global.source = parseInt(source.substr(4));
			} else if (isDuplicityVersion && source.startsWith('internal-net')) {
				global.source = parseInt(source.substr(13));
			}

			// Running raw event listeners
			try {
				rawEmitter.emit(name, payloadSerialized, source);
			} catch (e) {
				console.error('Unhandled error during running raw event listeners', e);
			}

			const listeners = emitter.listeners(name);
			if (listeners.length == 0) {
				global.source = null;
				return;
			}

			const payload = unpack(payloadSerialized) || [];
			if(!Array.isArray(payload)) {
				global.source = null;
				return;
			}

			// Running normal event listeners
			for (const listener of listeners) {
				try {
					const retval = listener.apply(null, payload);

					if (retval instanceof Promise) {
						(async () => {
							try {
								await retval;
							} catch (e) {
								console.error('Unhandled promise failure:', e);
							}
						})();
					}
				} catch (e) {
					global.printError('event `' + name + '\'', e);
				}
			}

			global.source = null;
		});
	});

	// Compatibility layer for legacy exports
	const exportsCallbackCache = {};
	const exportKey = isDuplicityVersion ? 'server_export' : 'export';
	const eventType = isDuplicityVersion ? 'Server' : 'Client';

	const getExportEventName = (resource, name) => {
		if(resource == "txAdmin") {
			resource = "monitor";
		}
		return `__cfx_export_${resource}_${name}`;
	}

	on(`on${eventType}ResourceStart`, (resource) => {
		if (resource === currentResourceName) {
			const numMetaData = GetNumResourceMetadata(resource, exportKey) || 0;

			for (let i = 0; i < numMetaData; i++) {
				const exportName = GetResourceMetadata(resource, exportKey, i);

				on(getExportEventName(resource, exportName), (setCB) => {
					if (global[exportName]) {
						setCB(global[exportName]);
					}
				});
			}
		}
	});

	on(`on${eventType}ResourceStop`, (resource) => {
		exportsCallbackCache[resource] = {};
	});

	// export invocation
	const createExports = () => {
		return new Proxy(() => { }, {
			get(t, k) {
				const resource = k;

				return new Proxy({}, {
					get(t, k) {
						if (!exportsCallbackCache[resource]) {
							exportsCallbackCache[resource] = {};
						}

						if (!exportsCallbackCache[resource][k]) {
							emit(getExportEventName(resource, k), (exportData) => {
								exportsCallbackCache[resource][k] = exportData;
							});

							if (!exportsCallbackCache[resource][k]) {
								throw new Error(`No such export ${k} in resource ${resource}`);
							}
						}

						return (...args) => {
							try {
								return exportsCallbackCache[resource][k](...args);
							} catch (e) {
								console.log(e);

								throw new Error(`An error occurred while calling export ${k} of resource ${resource} - see above for details`);
							}
						};
					},

					set() {
						throw new Error('cannot set values on an export resource');
					}
				});
			},

			apply(t, self, args) {
				if (args.length !== 2) {
					throw new Error('this needs 2 arguments');
				}

				const [exportName, func] = args;

				on(getExportEventName(currentResourceName, exportName), (setCB) => {
					setCB(func);
				});
			},

			set() {
				throw new Error('cannot set values on exports');
			}
		});
	};

	global.exports = createExports();

	const EXT_ENTITY = 41;
	const EXT_PLAYER = 42;

	global.NewStateBag = (es) => {
		return new Proxy({}, {
			get(_, k) {
				if (k === 'set') {
					return (s, v, r) => {
						const payload = msgpack_pack(v);
						SetStateBagValue(es, s, payload, payload.length, r);
					};
				}

				return GetStateBagValue(es, k);
			},

			set(_, k, v) {
				const payload = msgpack_pack(v);
				SetStateBagValue(es, k, payload, payload.length, isDuplicityVersion);
				return true; // If the set() method returns false, and the assignment happened in strict-mode code, a TypeError will be thrown.
			},
		});
	};

	global.GlobalState = NewStateBag('global');

	function getEntityStateBagId(entityGuid) {
		if (isDuplicityVersion || NetworkGetEntityIsNetworked(entityGuid)) {
			return `entity:${NetworkGetNetworkIdFromEntity(entityGuid)}`;
		} else {
			EnsureEntityStateBag(entityGuid);
			return `localEntity:${entityGuid}`;
		}
	}

	const entityTM = {
		get(t, k) {
			if (k === 'state') {
				const es = getEntityStateBagId(t.__data);

				if (isDuplicityVersion) {
					EnsureEntityStateBag(t.__data);
				}

				return NewStateBag(es);
			}

			return null;
		},

		set() {
			throw new Error('Not allowed at this time.');
		},

		__ext: EXT_ENTITY,

		__pack: () => {
			return String(NetworkGetNetworkIdFromEntity(this.__data));
		},

		__unpack: (data, t) => {
			const ref = NetworkGetEntityFromNetworkId(Number(data));
			return new Proxy({ __data: ref }, entityTM);
		},
	};

	const playerTM = {
		get(t, k) {
			if (k === 'state') {
				const pid = t.__data === -1 ? GetPlayerServerId(PlayerId()) : t.__data;

				const es = `player:${pid}`;

				return NewStateBag(es);
			}

			return null;
		},

		set() {
			throw new Error('Not allowed at this time.');
		},

		__ext: EXT_PLAYER,

		__pack: () => {
			return String(this.__data);
		},

		__unpack: (data, t) => {
			const ref = Number(data);
			return new Proxy({ __data: ref }, playerTM);
		},
	};

	global.Entity = (ent) => {
		if (typeof ent === 'number') {
			return new Proxy({ __data: ent }, entityTM);
		}

		return ent;
	};

	global.Player = (ent) => {
		if (typeof ent === 'number' || typeof ent === 'string') {
			return new Proxy({ __data: Number(ent) }, playerTM);
		}

		return ent;
	};

	if (!isDuplicityVersion) {
		global.LocalPlayer = Player(-1);
	}

	/*
	BEGIN
	https://github.com/errwischt/stacktrace-parser/blob/0121cc6e7d57495437818676f6b69be7d34c2fa7/src/stack-trace-parser.js

	MIT License

	Copyright (c) 2014-2019 Georg Tavonius

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
	*/

	const UNKNOWN_FUNCTION = '<unknown>';

	/**
	 * This parses the different stack traces and puts them into one format
	 * This borrows heavily from TraceKit (https://github.com/csnover/TraceKit)
	 */
	function parseStack(stackString) {
		const lines = stackString.split('\n');

		return lines.reduce((stack, line) => {
			const parseResult =
				parseChrome(line) ||
				parseWinjs(line) ||
				parseGecko(line) ||
				parseNode(line) ||
				parseJSC(line);

			if (parseResult) {
				stack.push(parseResult);
			}

			return stack;
		}, []);
	}

	const chromeRe = /^\s*at (.*?) ?\(((?:file|https?|blob|chrome-extension|native|eval|webpack|<anonymous>|\/|[a-z]:\\|\\\\).*?)(?::(\d+))?(?::(\d+))?\)?\s*$/i;
	const chromeEvalRe = /\((\S*)(?::(\d+))(?::(\d+))\)/;

	function parseChrome(line) {
		const parts = chromeRe.exec(line);

		if (!parts) {
			return null;
		}

		const isNative = parts[2] && parts[2].indexOf('native') === 0; // start of line
		const isEval = parts[2] && parts[2].indexOf('eval') === 0; // start of line

		const submatch = chromeEvalRe.exec(parts[2]);
		if (isEval && submatch != null) {
			// throw out eval line/column and use top-most line/column number
			parts[2] = submatch[1]; // url
			parts[3] = submatch[2]; // line
			parts[4] = submatch[3]; // column
		}

		const methodParts = (parts[1] || UNKNOWN_FUNCTION).split(/\./, 2);
		const typeName = methodParts.length == 2 ? (methodParts[0] + '.') : '';
		const methodName = methodParts[methodParts.length - 1];

		return {
			file: !isNative ? parts[2] : null,
			methodName,
			typeName,
			arguments: isNative ? [parts[2]] : [],
			lineNumber: parts[3] ? +parts[3] : null,
			column: parts[4] ? +parts[4] : null,
		};
	}

	const winjsRe = /^\s*at (?:((?:\[object object\])?.+) )?\(?((?:file|ms-appx|https?|webpack|blob):.*?):(\d+)(?::(\d+))?\)?\s*$/i;

	function parseWinjs(line) {
		const parts = winjsRe.exec(line);

		if (!parts) {
			return null;
		}

		return {
			file: parts[2],
			methodName: parts[1] || UNKNOWN_FUNCTION,
			arguments: [],
			lineNumber: +parts[3],
			column: parts[4] ? +parts[4] : null,
		};
	}

	const geckoRe = /^\s*(.*?)(?:\((.*?)\))?(?:^|@)((?:file|https?|blob|chrome|webpack|resource|\[native).*?|[^@]*bundle)(?::(\d+))?(?::(\d+))?\s*$/i;
	const geckoEvalRe = /(\S+) line (\d+)(?: > eval line \d+)* > eval/i;

	function parseGecko(line) {
		const parts = geckoRe.exec(line);

		if (!parts) {
			return null;
		}

		const isEval = parts[3] && parts[3].indexOf(' > eval') > -1;

		const submatch = geckoEvalRe.exec(parts[3]);
		if (isEval && submatch != null) {
			// throw out eval line/column and use top-most line number
			parts[3] = submatch[1];
			parts[4] = submatch[2];
			parts[5] = null; // no column when eval
		}

		return {
			file: parts[3],
			methodName: parts[1] || UNKNOWN_FUNCTION,
			arguments: parts[2] ? parts[2].split(',') : [],
			lineNumber: parts[4] ? +parts[4] : null,
			column: parts[5] ? +parts[5] : null,
		};
	}

	const javaScriptCoreRe = /^\s*(?:([^@]*)(?:\((.*?)\))?@)?(\S.*?):(\d+)(?::(\d+))?\s*$/i;

	function parseJSC(line) {
		const parts = javaScriptCoreRe.exec(line);

		if (!parts) {
			return null;
		}

		return {
			file: parts[3],
			methodName: parts[1] || UNKNOWN_FUNCTION,
			arguments: [],
			lineNumber: +parts[4],
			column: parts[5] ? +parts[5] : null,
		};
	}

	const nodeRe = /^\s*at (?:((?:\[object object\])?[^\\/]+(?: \[as \S+\])?) )?\(?(.*?):(\d+)(?::(\d+))?\)?\s*$/i;

	function parseNode(line) {
		const parts = nodeRe.exec(line);

		if (!parts) {
			return null;
		}

		const methodParts = (parts[1] || UNKNOWN_FUNCTION).split(/\./, 2);
		const typeName = methodParts.length == 2 ? (methodParts[0] + '.') : '';
		const methodName = methodParts[methodParts.length - 1];

		return {
			file: parts[2],
			typeName,
			methodName,
			arguments: [],
			lineNumber: +parts[3],
			column: parts[4] ? +parts[4] : null,
		};
	}
	// END	
})(this || globalThis);
