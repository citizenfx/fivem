export interface IShellCommands {
  register(id: string, handler: Function): void;
  invoke<T extends any[]>(id: string, ...args: T): unknown;
}

export const ShellCommands = new class ShellCommands {
  private registry: Record<string, Function> = {};

  register(id: string, handler: Function) {
    this.registry[id] = handler;
  }

  invoke<T extends any[]>(id: string, ...args: T) {
    const handler = this.registry[id];
    if (handler) {
      return handler(...args);
    }
  }
}();
