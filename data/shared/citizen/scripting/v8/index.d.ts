interface CitizenConsoleConstructor {
    new(): void
}

interface Console {
    Console: NodeJS.ConsoleConstructor;
    assert(value: any, message?: string, ...optionalParams: any[]): void;
    dir(obj: any, options?: NodeJS.InspectOptions): void;
    error(message?: any, ...optionalParams: any[]): void;
    info(message?: any, ...optionalParams: any[]): void;
    log(message?: any, ...optionalParams: any[]): void;
    time(label: string): void;
    timeEnd(label: string): void;
    trace(message?: any, ...optionalParams: any[]): void;
    warn(message?: any, ...optionalParams: any[]): void;
}