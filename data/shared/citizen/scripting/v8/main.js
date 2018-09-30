// CFX JS runtime
/// <reference path="./natives_blank.d.ts" />
/// <reference path="./natives_server.d.ts" />

const EXT_FUNCREF = 10;
const EXT_LOCALFUNCREF = 11;

(function (global) {
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

	function refFunctionPacker(refFunction) {
		const ref = Citizen.makeRefFunction(refFunction);

		return ref;
	}

	function refFunctionUnpacker(refSerialized) {
		const fnRef = Citizen.makeFunctionReference(refSerialized);
	
		return function (...args) {
			return unpack(fnRef(pack(args)));
		};
	}

	codec.addExtPacker(EXT_FUNCREF, Function, refFunctionPacker);
	codec.addExtUnpacker(EXT_FUNCREF, refFunctionUnpacker);
	codec.addExtUnpacker(EXT_LOCALFUNCREF, refFunctionUnpacker);

	/**
	 * Deletes ref function
	 * 
	 * @param {int} ref
	 */
	Citizen.setDeleteRefFunction(function(ref) {
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
	Citizen.setCallRefFunction(function(ref, argsSerialized) {
		if (!refFunctionsMap.has(ref)) {
			console.error('Invalid ref call attempt:', ref);

			return pack([]);
		}

		return pack([refFunctionsMap.get(ref).callback(...unpack(argsSerialized))]);
	});

	/**
	 * Duplicates ref function
	 * 
	 * @param {int} ref
	 */
	Citizen.setDuplicateRefFunction(function(ref) {
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
	const netSafeEventNames = new Set(['playerDropped', 'playerConnecting']);

	// Raw events
	global.addRawEventListener = rawEmitter.on.bind(rawEmitter);
	global.addRawEventHandler = global.addRawEventListener;

	// Client events
	global.addEventListener = (name, callback, netSafe = false) => {
		if (netSafe) {
			netSafeEventNames.add(name);
		}

		emitter.on(name, callback);
	};
	global.on = global.addEventListener;

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

		TriggerEventInternal(name, dataSerialized, dataSerialized.length);
	};

	global.TriggerEvent = global.emit;

	if (IsDuplicityVersion()) {
		global.emitNet = (name, source, ...args) => {
			const dataSerialized = pack(args);
	
			TriggerClientEventInternal(name, source, dataSerialized, dataSerialized.length);
		};

		global.TriggerClientEvent = global.emitNet;
	} else {
		global.emitNet = (name, ...args) => {
			const dataSerialized = pack(args);
	
			TriggerServerEventInternal(name, dataSerialized, dataSerialized.length);
		};

		global.TriggerServerEvent = global.emitNet;
	}

	/**
	 * @param {string} name
	 * @param {UInt8Array} payloadSerialized
	 * @param {string} source
	 */
	Citizen.setEventFunction(async function(name, payloadSerialized, source) {
		global.source = source;

		if (source.startsWith('net')) {
			if (emitter.listeners(name).length > 0 && !netSafeEventNames.has(name)) {
				console.error(`Event ${name} was not safe for net`);

				global.source = null;
				return;
			}

			global.source = parseInt(source.substr(4));
		}

		const payload = unpack(payloadSerialized) || [];
		const listeners = emitter.listeners(name);

		if (listeners.length === 0 || !Array.isArray(payload)) {
			global.source = null;
			return;
		}

		// Running normal event listeners
		for (const listener of listeners) {
			const retval = listener.apply(null, payload);

			if (retval instanceof Promise) {
				await retval;
			}
		}

		// Running raw event listeners
		try {
			rawEmitter.emit(name, payloadSerialized, source);
		} catch(e) {
			console.error('Unhandled error during running raw event listeners', e);
		}

		global.source = null;
	});

	// Compatibility layer for legacy exports
	const exportsCallbackCache = {};
	const exportKey = (IsDuplicityVersion()) ? 'server_export' : 'export';
	const eventType = (IsDuplicityVersion() ? 'Server' : 'Client');

	const getExportEventName = (resource, name) => `__cfx_export_${resource}_${name}`;

	on(`on${eventType}ResourceStart`, (resource) => {
		if (resource === GetCurrentResourceName()) {
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
		return new Proxy(() => {}, {
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
								const result = exportsCallbackCache[resource][k](...args);

								if (Array.isArray(result) && result.length === 1) {
									return result[0];
								}

								return result;
							} catch (e) {
								console.error(e);

								throw new Error(`An error happened while calling export ${k} of resource ${resource} - see above for details`);
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

				const [ exportName, func ] = args;

				on(getExportEventName(GetCurrentResourceName(), exportName), (setCB) => {
					setCB(func);
				});
			},

			set() {
				throw new Error('cannot set values on exports');
			}
		});
	};

	global.exports = createExports();
})(this || window);
