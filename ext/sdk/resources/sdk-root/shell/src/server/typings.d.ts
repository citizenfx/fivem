declare var openDevTools: () => void;
declare var invokeNative: (native: string, ...arg: string[]) => void;

declare var on: (event: string, cb: <T>(T) => void) => void;
declare var emit: (event: string, data?: any) => void;
declare var GetConvar: (name: string) => string;

declare var requireTheiaBackend: () => any;
