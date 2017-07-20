// CFX JS RUNTIME DEFINITIONS

interface Console {
    assert(value: any, ...optionalParams: any[]): void
    error(...optionalParams: any[]): void
    info(...optionalParams: any[]): void
    log(...optionalParams: any[]): void
    time(label: string): void
    timeEnd(label: string): void
    trace(...optionalParams: any[]): void
    warn(...optionalParams: any[]): void
}

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
    ResultAsVector;

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

    makeRefFunction(refFunction: Function): string
}

declare const console: Console;

declare const Citizen: CitizenInterface;

declare function addRawEventListener(eventName: string, callback: Function): void

declare function addEventListener(eventName: string, callback: Function): void
declare function on(eventName: string, callback: Function): void

declare function addNetEventListener(eventName: string, callback: Function): void
declare function onNet(eventName: string, callback: Function): void

declare function emit(eventName: string, ...args: any[]): void
declare function emitNet(eventName: string, ...args: any[]): void

declare function setTick(callback: Function): void
