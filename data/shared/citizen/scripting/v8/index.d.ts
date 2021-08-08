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

declare var Citizen: CitizenInterface;

declare function addRawEventListener(eventName: string, callback: Function): void

declare function addEventListener(eventName: string, callback: Function, netSafe?: boolean): void
declare function on(eventName: string, callback: Function): void
declare function AddEventHandler(eventName: string, callback: Function): void

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
declare function PerformHttpRequest(url: string, cb: Function, method?: string, data?: string, headers?: { [key: string]: string }, options?: object): void

declare function SendNUIMessage(data: any): void

declare function emitNet(eventName: string, target: number|string, ...args: any[]): void
declare function TriggerClientEvent(eventName: string, target: number|string, ...args: any[]): void
declare function TriggerLatentClientEvent(eventName: string, target: number|string, bps: number, ...args: any[]): void

declare function removeEventListener(eventName: string, callback: Function): void

declare function setTick(callback: Function): number
declare function clearTick(callback: number): void

declare function NewStateBag(name: string) : StateBagInterface;
declare function Entity(entity: number): EntityInterface
declare var GlobalState : StateBagInterface
declare function Player(entity: number|string): EntityInterface

declare var exports: any;

declare var source: number;
