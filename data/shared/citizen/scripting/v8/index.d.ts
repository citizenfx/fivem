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

interface StateBagInterface {
    [key: string]: any;
    set(key: string, value: any, replicated: boolean): void
}

interface EntityInterface {
    state: StateBagInterface
}

interface CitizenInterface {
    trace(...args: string[]): void
    setTickFunction(callback: Function): void
    setEventFunction(callback: Function): void

    setCallRefFunction(callback: Function): void
    setDeleteRefFunction(callback: Function): void
    setDuplicateRefFunction(callback: Function): void
    canonicalizeRef(ref: number): string
    invokeFunctionReference(ref: string, args: Uint8Array): Uint8Array

    getTickCount(): number
    invokeNative<T = void>(hash: string, ...args: InputArgument[]): T
    startProfiling(name?: string): void
    stopProfiling(name?: string): {}

    pointerValueIntInitialized(): IntPtrInitialized
    pointerValueFloatInitialized(): FloatPtrInitialized
    pointerValueInt(): IntPtr
    pointerValueFloat(): FloatPtr
    pointerValueVector(): VectorPtr
    returnResultAnyway(): ReturnResultAnyway
    resultAsInteger(): ResultAsInteger
    resultAsFloat(): ResultAsFloat
    resultAsString(): ResultAsString
    resultAsVector(): ResultAsVector
    resultAsLong(): ResultAsLong
    resultAsObject(): ResultAsObject

    makeRefFunction(refFunction: Function): string
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

declare function addRawEventListener(eventName: string, callback: Function): void
declare function setMaxRawEventListeners(max: number): void

declare function addEventListener(eventName: string, callback: Function, netSafe?: boolean): void
declare function on(eventName: string, callback: Function): void
declare function AddEventHandler(eventName: string, callback: Function): void
declare function setMaxEventListeners(max: number): void

declare function addNetEventListener(eventName: string, callback: Function): void
declare function onNet(eventName: string, callback: Function): void

declare function emit(eventName: string, ...args: any[]): void
declare function TriggerEvent(eventName: string, ...args: any[]): void

declare function emitNet(eventName: string, ...args: any[]): void
declare function TriggerServerEvent(eventName: string, ...args: any[]): void
declare function TriggerLatentServerEvent(eventName: string, bps: number, ...args: any[]): void

declare function getPlayerIdentifiers(player: number|string): string[]
declare function getPlayerTokens(player: number|string): string[]
declare function getPlayers(): string[]

declare function SendNUIMessage(data: any): void

declare function emitNet(eventName: string, target: number|string, ...args: any[]): void
declare function TriggerClientEvent(eventName: string, target: number|string, ...args: any[]): void
declare function TriggerLatentClientEvent(eventName: string, target: number|string, bps: number, ...args: any[]): void

declare function removeEventListener(eventName: string, callback: Function): void

declare function setTimeout<T extends any[]>(callback: (...args: T) => void, ms?: number, ...args: T): CitizenTimer;
declare function clearTimeout(timeout: CitizenTimer): void;

declare function setInterval<T extends any[]>(callback: (...args: T) => void, ms?: number, ...args: T): CitizenTimer;
declare function clearInterval(interval: CitizenTimer): void;

declare function setImmediate<T extends any[]>(callback: (...args: T) => void, ...args: T): CitizenImmediate;
declare function clearImmediate(immediate: CitizenImmediate): void;

declare function setTick(callback: Function): number
declare function clearTick(callback: number): void

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
