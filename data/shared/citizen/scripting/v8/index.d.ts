/// <reference path="./natives_universal.d.ts"/>

interface IntPtrInitialized {}
interface FloatPtrInitialized {}
interface IntPtr {}
interface FloatPtr {}
interface VectorPtr {}
interface ReturnResultAnyway {}
interface ResultAsInteger {}
interface ResultAsFloat {}
interface ResultAsString {}
interface ResultAsVector {}
interface ResultAsLong {}
interface ResultAsObject {}

type InputArgument =
    string |
    number |
    IntPtrInitialized |
    FloatPtrInitialized |
    IntPtr |
    FloatPtr |
    VectorPtr |
    ReturnResultAnyway |
    ResultAsInteger |
    ResultAsFloat |
    ResultAsString |
    ResultAsVector |
    ResultAsLong |
    ResultAsObject;

/**
  * NOTE: The state bag system will always have to deserialize any access from
  * it's internal msgpack representation, where possible you should cache this
  * and state bag listeners to update the value.
  */
interface StateBagInterface {
    [key: string]: any;
    set(key: string, value: any, replicated: boolean): void
}

interface EntityInterface {
    state: StateBagInterface
}

interface CitizenInterface {
    /**
    * https://developer.mozilla.org/en-US/docs/Web/API/console/log_static
    */
    trace(...args: string[]): void
    /**
      * Internal: This defines the function v8 will call for the next tick
      *
      * Calling this in user code does nothing.
      */
    setTickFunction(callback: Function): void
    /**
      * Internal: This defines the function that the internal event runtime will call
      * when the ScRT receives an event.
      *
      * Calling this in end user code does nothing.
      */
    setEventFunction(callback: Function): void

    /**
      * Internal: This defines the function that v8 will call whenever another script
      * calls a function into the current runtime.
      *
      * Calling this in end user code does nothing.
      */
    setCallRefFunction(callback: Function): void
    /**
      * Internal: This defined the function that v8 will call whenever a function
      * is no longer refered to by another ScRT.
      *
      * Calling this in end user code does nothing.
      */
    setDeleteRefFunction(callback: Function): void
    /**
      * Internal: This defines the function that v8 will call whenever a resource
      * gets a reference to a function
      */
    setDuplicateRefFunction(callback: Function): void
    /**
      * Internal: Converts a ref number into a formatted reference function call,
      * i.e. `[resource_name]:[instance_id]:[reference_id]`
      */
    canonicalizeRef(ref: number): string
    /**
      * Makes a function reference and returns the canonicalized version of the ref
      */
    makeRefFunction(refFunction: Function): string

    /**
      * Returns the current tick time in milliseconds
      */
    getTickCount(): number

    /**
      * Invokes a native by a string version of its hash.
      *
      * This is used internally with safe guards to prevent unexpected behavior,
      * by directly invoking this you should take note:
      * 1. Passing a string to a function that expects a hash will not automatically
      * convert them into a hash.
      * 2. Natives that return an `object` will now return a msgpack string
      * 3. Passing anything that is not a string to a function that expects a string
      * will not work properly.
      * 4. Passing a number to a native that expects a float will not automatically
      * convert it to a floating point number, if you use a bundler and set your number
      * to `x.0` it's likely that your bundler will automatically convert it to `0`
      * 5. Any native that takes a function as an argument will not automatically
      * convert the function into an ScRT function reference, you will have to do
      * this manually with {@link CitizenInterface.makeFunctionReference}.
      *
      * @param hash - the string version of the native hash
      *
      * @example For a native like https://docs.fivem.net/natives/?_0x06843DA7060A026B
      * ```ts
      * // SET_ENTITY_COORDS
      * Citizen.invokeNative("0x06843DA7060A026B", entity, 123.55, 543.33, 154.2)
      */
    invokeNative<T = void>(hash: string, ...args: InputArgument[]): T

    /**
      * You should see {@link invokeNative} for possible pit falls when using
      * direct invokes for these natives.
      *
      * Since JS can only support 53 bits of precision we can't use the full
      * 64 bits of a normal hex address so we have to split the address into the
      * upper and lower 32 bits and reconstruct them into a 64bit integer in C++
      *
      * This is mostly used by internal calls as it can be slightly faster.
      */
    invokeNativeByHash<T = void>(high32Bits: number, low32Bits: number, ...args: InputArgument[]): T;

    /**
      * Starts the v8 profiling API
      *
      * NOTE: Due to how FiveM v8 is setup, *all* resources will be included in
      * the profiler, since they all use the same v8 isolate.
      */
    startProfiling(profileTitle?: string): void
    /**
      * Stops the v8 profile and outputs it into a `.cpuprofile` file inside of
      * your citizen folder which should be openable in chrome.
      *
      * For clients this will be in `%localappdata%/FiveM/FiveM.app/`, on servers
      * this will be whatever directory your `FXServer.exe` is in.
      */
    stopProfiling(profileTitle?: string): string

    /**
      * Takes a heap snapshot v8 memory space, this outputs it into a `snap.heapsnapshot`.
      *
      * NOTE: Due to how FiveM v8 is setup, *all* JS resources will be included
      * in the heap snapshot.
      */
    snap(): void;

    /**
      * Submits a boundary stack to the runtime, this is used internally to help
      * with stack traces across ScRT bounds
      */
    submitBoundaryStart(boundary: string): void;

    /**
      * Submits a boundary end, ending the current stack frame internally
      */
    submitBoundaryEnd(boundary: string): void;

    /**
      * Can be thought as a "reference" in lower level languages, this will make
      * a pointer internally that will get initialized to {@link num} for use in
      * native calls where they take a pointer.
      *
      * The return type of the native this is called in will be converted into an
      * array if there are multiple return values.
      *
      * @param num - the integer value this should be initialized to
      * @example
      * ```ts
      * // the number that is returned by DeleteEntity after this is called should
      * // be `0`
      * function DeleteEntity(entity: number): number {
      *   return Citizen.invokeNative("0xAE3CBE5BF394C9C9", Citizen.pointerValueIntInitialized(entity));
      * }
      * ```
      */
    pointerValueIntInitialized(num: number): IntPtrInitialized

    /**
      * Can be thought as a "reference" in lower level languages.
      *
      * The return type of the native this is called in will be converted into an
      * array if there are multiple return values.
      *
      * @param num - the number this float should be initialized to
      */
    pointerValueFloatInitialized(num: number): FloatPtrInitialized

    /**
      * Can be thought as a "reference" in lower level languages, internally this
      * will make a pointer that the scripting interface can write to.
      * 
      * The return type of the native this is called in will be converted into an
      * array if there are multiple return values.
      */
    pointerValueInt(): IntPtr

    /**
      * Can be thought as a "reference" in lower level languages, internally this
      * will make a pointer that the scripting interface can write to.
      * 
      * The return type of the native this is called in will be converted into an
      * array if there are multiple return values.
      */
    pointerValueFloat(): FloatPtr

    /**
      * Can be thought as a "reference" in lower level languages, internally this
      * will make a pointer that the scripting interface can write to.
      * 
      * The return type of the native this is called in will be converted into an
      * array if there are multiple return values.
      *
      */
    pointerValueVector(): VectorPtr

    /**
      * Tells the scripting runtime to return the results of a native call if it has any
      */
    returnResultAnyway(): ReturnResultAnyway

    /**
      * Tells the scripting runtime to return the results of the native invoke as
      * an integer.
      */
    resultAsInteger(): ResultAsInteger

    /**
      * Tells the scripting runtime to return the results of the native invoke as
      * a long.
      */
    resultAsLong(): ResultAsLong

    /**
      * Tells the scripting runtime to return the results of the native invoke as
      * a floating point value.
      */
    resultAsFloat(): ResultAsFloat

    /**
      * Tells the scripting runtime to return the results of the native invoke as
      * a string value.
      */
    resultAsString(): ResultAsString

    /**
      * Tells the scripting runtime to return the results of the native invoke as
      * a vector value.
      */
    resultAsVector(): ResultAsVector

    /**
      * Tells the scripting runtime to return the results of the native invoke as
      * an object.
      */
    resultAsObject2(): ResultAsObject

}

interface CitizenTimer {
    ref(): CitizenTimer,
    unref(): CitizenTimer,
    hasRef(): boolean,
    refresh(): CitizenTimer,
    [Symbol.toPrimitive](): number,
}

type CitizenImmediate = Omit<CitizenTimer, 'refresh'>;

declare var Citizen: CitizenInterface;


/**
  * Packs the specified {@link data} into a msgpack buffer
  */
declare function msgpack_pack<T>(data: T): Uint8Array;

/**
  * Unpacks the specified {@link packedArgs} into a JS Object.
  */
declare function msgpack_unpack<T = any>(packedArgs: Uint8Array): T;

declare type EventCallback = (...args: any[]) => void | Promise<void>;

declare type RawEventCallback = (data: Uint8Array, source: string) => void | Promise<void>;

/**
  * Registers the specified {@link eventName} to be safe to receive across the
  * network
  */
declare function RegisterNetEvent(eventName: string): void;

/**
  * Declares a function that will be called before any msgpack deserialization
  * happens. Useful if you want to use something like protobuf.
  *
  * NOTE: You will have to manually call {@link RegisterNetEvent} to register this
  * as a valid net event recepient
  *
  * NOTE: The `source` that gets sent to callback is a unmodified version of the source,
  * you will need to remove the `net:` or `internal-net:` prefix, there is no
  * guarantee that more prefixes will not be added in the future, your logic
  * should take that into consideration.
  * @param eventName - the name to listen for the event
  * @param callback - the callback to call for the raw event
  * @param [netSafe=false] - If the event is network safe, by default this will be false.
  */
declare function addRawEventListener(eventName: string, callback: RawEventCallback, netSafe: boolean = false): void

/**
  * Declares a function that will be called before any msgpack deserialization
  * happens. Useful if you want to use something like protobuf.
  *
  * NOTE: You will have to manually call {@link RegisterNetEvent} to register this
  * as a valid net event recepient
  *
  * NOTE: The `source` that gets sent to callback is a unmodified version of the source,
  * you will need to remove the `net:` or `internal-net:` prefix, there is no
  * guarantee that more prefixes will not be added in the future, your logic
  * should take that into consideration.
  * @param eventName - the name to listen for the event
  */
declare function addRawEventHandler(eventName: string, callback: RawEventCallback): void

/**
  * Adds a raw event listener for the specified {@link eventName} and automatically
  * registers the event to be network safe.
  *
  * @param eventName - the event name we should listen to
  * @param callback - the function that should be called whenever an event is triggered
  */
declare function onRawNet(eventName: string, callback: RawEventCallback): void;

/**
  * Removes the event listener with the specified {@link eventName} and {@link callback}
  */
declare function removeRawEventListener(eventName: string, callback: EventCallback): void

/**
  * Sets the maximum amount of listeners all raw events can have.
  *
  * If you re-use the same event for *all* of your events you will likely need
  * to increase this past the default `10`, otherwise this will error and notify
  * you that you might accidentally be leaking memory.
  *
  * @param [max=10] the maximum amount of raw events that can be registered, by default this is 10.
  */
declare function setMaxRawEventListeners(max: number): void

/**
  * Sends an event across the network to the client/server, if the client/server
  * hasn't registered the event as network safe it will automatically be dropped.
  *
  * WARNING: Raw net emits should only be used with raw net events, like {@link onRawNetEvent}.
  * Using these on a regular event will return without calling any of the regular event emitters
  *
  * NOTE: This is a sequential channel internally, this means that packets will be
  * sent/received in the same order, which also means that if you send large net
  * events the client will *have* to wait to receive those events.
  *
  * Sending too large of events can cause the client to timeout.
  *
  * @param eventName - the event name to trigger under
  * @param source - on the server this defines the player to send the event to, or `-1` for everyone.
  * @param data - the data to send to the client/server
  * @throws this will throw if `data` isn't an instance of Uint8Array
  */
declare function emitRawNet(eventName: string, data: Uint8Array): void
declare function emitRawNet(eventName: string, source: number, data: Uint8Array): void

/**
  * Sends an event across the local event system that other resources can listen
  * to.
  *
  * WARNING: Raw emits should only be used with raw events, like {@link addRawEventHandler}, and {@link addRawEventListener}.
  * Using these on a regular event will return without calling any of the regular event emitters
  *
  * @param eventName - the event name to trigger under
  * @param [args=undefined] - the arguments to send
  */
declare function emitRaw(eventName: string, data: Uint8Array): void;

/**
  * Adds an event listener for the specified {@link eventName}, and optionally
  * sets the event to be able to listen to net events.
  *
  * @param eventName - the event name we should listen to
  * @param callback - the function that should be called whenever an event is triggered
  * @param [netSafe=false] - wheter or not this should be registered to be safe for the network.
  */
declare function addEventListener(eventName: string, callback: EventCallback, netSafe?: boolean): void

/**
  * Adds an event listener for the specified {@link eventName}
  *
  * @param eventName - the event name we should listen to
  * @param callback - the function that should be called whenever an event is triggered
  */
declare function on(eventName: string, callback: EventCallback): void

/**
  * Adds an event listener for the specified {@link eventName}
  *
  * @param eventName - the event name we should listen to
  * @param callback - the function that should be called whenever an event is triggered
  */
declare function AddEventHandler(eventName: string, callback: EventCallback): void

/**
  * Sets the maximum amount of events that can be registered to a specified `eventName`
  *
  * If you re-use the same event for *all* of your events you will likely need
  * to increase this past the default `10`, otherwise this will error and notify
  * you that you might accidentally be leaking memory.
  *
  * @param [max=10] - The max amount of listeners that should be able to listen to an event
  */
declare function setMaxEventListeners(max: number): void

/**
  * Adds an event listener for the specified {@link eventName} and automatically
  * registers the event to be network safe.
  *
  * @param eventName - the event name we should listen to
  * @param callback - the function that should be called whenever an event is triggered
  */
declare function addNetEventListener(eventName: string, callback: EventCallback): void

/**
  * Adds an event listener for the specified {@link eventName} and automatically
  * registers the event to be network safe.
  *
  * @param eventName - the event name we should listen to
  * @param callback - the function that should be called whenever an event is triggered
  */
declare function onNet(eventName: string, callback: EventCallback): void

/**
  * Removes the event listener with the specified {@link eventName} and {@link callback}
  */
declare function removeEventListener(eventName: string, callback: EventCallback): void

/**
  * Sends an event across the local event system that other resources can listen
  * to.
  *
  * @param eventName - the event name to trigger under
  * @param [args=undefined] - the arguments to send
  */
declare function emit(eventName: string, ...args: any[]): void

/**
  * Sends an event across the local event system that other resources can listen
  * to.
  *
  * @param eventName - the event name to trigger under
  * @param [args=undefined] - the arguments to send
  */
declare function TriggerEvent(eventName: string, ...args: any[]): void

/**
  * Sends an event across the network to the client/server, if the client/server
  * hasn't registered the event as network safe it will automatically be dropped.
  *
  * NOTE: This is a sequential channel internally, this means that packets will be
  * sent/received in the same order, which also means that if you send large net
  * events the client will *have* to wait to receive those events.
  *
  * Sending too large of events can cause the client to timeout.
  *
  * @param eventName - the event name to trigger under
  * @param source - on the server this defines the player to send the event to, or `-1` for everyone.
  * @param [args=undefined] - the arguments to send
  */
declare function emitNet(eventName: string, ...args: any[]): void
declare function emitNet(eventName: string, source: number, ...args: any[]): void



/**
  * NOTE: This function is only available on the client, you should generally use
  * {@link emitNet} if you want to share code
  *
  * Sends an event across the network to the server, if the server
  * hasn't registered the event as network safe it will automatically be dropped.
  *
  * @param eventName - the event name to trigger under
  * @param [args=undefined] - the arguments to send
  */
declare function TriggerServerEvent(eventName: string, ...args: any[]): void

/**
  * Sends an event across the network to the server, if the server
  * hasn't registered the event as network safe it will automatically be dropped.
  *
  * This event will be "rate limited" and will only send {@link bps} bytes per second
  * to the server until it eventually sends the entire payload
  *
  * This should only be used for extremely large events.
  *
  * @param eventName - the event name to trigger under
  * @param bps - the amount of bytes to send to the server per second, setting this to `0` will default to `25000`
  * @param [args=undefined] - the arguments to send
  */
declare function TriggerLatentServerEvent(eventName: string, bps: number, ...args: any[]): void

/**
  * Sends an event across the network to the client/server, if the client/server
  * hasn't registered the event as network safe it will automatically be dropped.
  *
  * NOTE: This is a sequential channel internally, this means that packets will be
  * sent/received in the same order, which also means that if you send large net
  * events the client will *have* to wait to receive those events.
  *
  * Sending too large of events can cause the client to timeout.
  *
  * @param eventName - the event name to trigger under
  * @param [args=undefined] - the arguments to send
  */
declare function TriggerClientEvent(eventName: string, target: number|string, ...args: any[]): void

/**
  * Sends an event across the network to the server, if the server
  * hasn't registered the event as network safe it will automatically be dropped.
  *
  * This event will be "rate limited" and will only send {@link bps} bytes per second
  * to the server until it eventually sends the entire payload
  *
  * This should only be used for extremely large events as these have book keeping
  * overhead.
  *
  * @param eventName - the event name to trigger under
  * @param bps - the amount of bytes to send to the server per second, setting this to `0` will default to `25000`
  * @param [args=undefined] - the arguments to send
  */
declare function TriggerLatentClientEvent(eventName: string, target: number|string, bps: number, ...args: any[]): void

/**
  * You can find a list of supported identifiers here:
  * https://docs.fivem.net/docs/scripting-reference/runtimes/lua/functions/GetPlayerIdentifiers/
  * @returns the specified {@link player}'s linked identifiers
  */
declare function getPlayerIdentifiers(player: number|string): string[]
/**
  * @returns hardware tokens for the specified player, this will be unique across servers
  */
declare function getPlayerTokens(player: number|string): string[]
/**
  * @returns a list of all currently online players
  */
declare function getPlayers(): string[]

/**
  * Converts the {@link data} to a JSON object and sends it to the resources NUI
  *
  * @param data - an object that can be converted into JSON
  */
declare function SendNUIMessage(data: object): void


/**
  * Sets {@link callback} to be called in {@link ms} millseconds, unless canceled
  * by a call to {@link clearTimeout}
  * @param callback - the callback to execute once the timeout is up
  * @param ms - The time in milliseconds to wait until we call the function, not setting this will have it act like {@link setImmediate}
  * @param args - the arguments to pass through the callback.
  */
declare function setTimeout<T extends any[]>(callback: (...args: T) => void, ms?: number, ...args: T): CitizenTimer;
/**
  * Clears the timeout with the specified timer id.
  */
declare function clearTimeout(timeout: CitizenTimer): void;


/**
  * Sets {@link callback} to be called every {@link ms} millseconds, unless canceled
  * by a call to {@see clearInterval}
  *
  * This function will *always* be called once every {@link ms} milliseconds, even
  * if its awaited.
  *
  * If you need something that wait for the last promise to finish you should use
  * {@link setTick}
  *
  * @param callback - the callback to execute
  * @param ms - The time in milliseconds to wait until we call the function again
  * @param args - the arguments to pass through the callback.
  */
declare function setInterval<T extends any[]>(callback: (...args: T) => void, ms?: number, ...args: T): CitizenTimer;
/**
  * Clears the interval with the specified timer id.
  */
declare function clearInterval(interval: CitizenTimer): void;

/**
  * Sets {@link callback} to be called on the next tick.
  *
  * @param callback - the callback to execute
  * @param args - the arguments to pass through the callback.
  */
declare function setImmediate<T extends any[]>(callback: (...args: T) => void, ...args: T): CitizenImmediate;
/**
  * Clears the immediate call
  */
declare function clearImmediate(immediate: CitizenImmediate): void;

/**
  * Sets {@link callback} to be called every game frame
  *
  * If the {@link callback} is declared as async the tick will not be called
  * again until anything that is `await`ed inside of the tick finishes resolving.
  *
  * @param callback - the callback to execute
  */
declare function setTick(callback: () => void | Promise<void>): number
/**
  * Clears the tick
  */
declare function clearTick(callback: number): void

/**
  * Prints the specified error with its appropriate stack trace
  */
declare function printError(where: string, e: Error | unknown): void;

declare function NewStateBag(name: string) : StateBagInterface;
declare function Entity(entity: number): EntityInterface
declare var GlobalState : StateBagInterface
declare function Player(entity: number|string): EntityInterface
declare var LocalPlayer : EntityInterface

interface CitizenExports {
    (exportKey: string | number, exportFunction: Function): void;
    [resourceName: string] : {
        [exportKey: string | number]: Function
    };
}

declare var exports: CitizenExports;

/**
  * Used inside of {@see onNet} on the server, this will be the source of the caller
  *
  * You should cache this value if you plan on using it across await points as it
  * will be changed whenever another event gets called.
  */
declare var source: number;

// Commented methods are not implemented yet
interface Console {
    assert(condition?: boolean, ...data: any[]): void;
    // clear(): void;
    count(label?: string): void;
    countReset(label?: string): void;
    debug(...data: any[]): void;
    dir(item?: any, options?: any): void;
    // dirxml(...data: any[]): void;
    error(...data: any[]): void;
    // group(...data: any[]): void;
    // groupCollapsed(...data: any[]): void;
    // groupEnd(): void;
    info(...data: any[]): void;
    log(...data: any[]): void;
    // table(tabularData?: any, properties?: string[]): void;
    time(label?: string): void;
    timeEnd(label?: string): void;
    // timeLog(label?: string, ...data: any[]): void;
    // timeStamp(label?: string): void;
    trace(...data: any[]): void;
    warn(...data: any[]): void;
}

declare var console: Console;
