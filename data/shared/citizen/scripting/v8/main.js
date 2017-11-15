// CFX JS runtime

const EXT_FUNCREF = 10;

(function (global) {
	let refIndex = 0;
	const nextRefIdx = () => refIndex++;
	const refFunctionsMap = new Map();

	const codec = msgpack.createCodec({
		uint8array: true,
		preset: false
	});

	const pack = data => msgpack.encode(data, { codec });
	const unpack = data => msgpack.decode(data, { codec });

	/**
	 * @param {Function} refFunction
	 * @returns {string}
	 */
	Citizen.makeRefFunction = (refFunction) => {
		const ref = nextRefIdx();

		refFunctionsMap.set(ref, refFunction);

		return Citizen.canonicalizeRef(ref);
	};

	function refFunctionPacker(refFunction) {
		const ref = Citizen.makeRefFunction(refFunction);

		return ref;
	}

	function refFunctionUnpacker(refSerialized) {
		const ref = msgpack.decode(refSerialized);

		return function (...args) {
			return unpack(Citizen.invokeFunctionReference(ref, pack(args)));
		};
	}

	codec.addExtPacker(EXT_FUNCREF, Function, refFunctionPacker);
	codec.addExtUnpacker(EXT_FUNCREF, refFunctionUnpacker);

	/**
	 * Deletes ref function
	 * 
	 * @param {int} ref
	 */
	Citizen.setDeleteRefFunction(function(ref) {
		refFunctionsMap.delete(ref);
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

			return pack({});
		}

		return pack(refFunctionsMap.get(ref)(...unpack(argsSerialized)));
	});

	/**
	 * Duplicates ref function
	 * 
	 * @param {int} ref
	 */
	Citizen.setDuplicateRefFunction(function(ref) {
		if (refFunctionsMap.has(ref)) {
			const refFunction = refFunctionsMap.get(ref);
			const newRef = nextRefIdx();

			refFunctionsMap.set(newRef, refFunction);

			return newRef;
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

	global.emit = (name, ...args) => {
		const dataSerialized = pack(args);

		Citizen.invokeNative('0x91310870', name, dataSerialized, dataSerialized.length);
	};
	global.emitNet = (name, ...args) => {
		const dataSerialized = pack(args);

		Citizen.invokeNative('0x7fdd1128', name, dataSerialized, dataSerialized.length);
	};

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
})(this || window);
