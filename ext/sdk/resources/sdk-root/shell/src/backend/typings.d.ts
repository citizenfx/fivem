declare var openDevTools: () => void;
declare var invokeNative: (native: string, ...arg: string[]) => unknown;

declare var on: <T extends any[]>(event: string, cb: (...args: T) => void) => void;
declare var RemoveEventHandler: <T extends any[]>(event: string, cb: (...args: T) => void) => void;
declare var emit: <T extends any[]>(event: string, ...data?: T) => void;
declare var GetConvar: (name: string) => string;

/**
 * @see ../../index.js
 */
declare var nativeRequire: (path: string) => any;
