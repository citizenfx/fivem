import { IDisposableObject } from "fxdk/base/disposable";
import { getExpireAt, NotificationState } from "store/NotificationState";

export type IShellCommandId =
  | string
  | symbol;

export interface IDynamicShellCommand extends IDisposableObject {
  id: symbol,
}
export interface IShellCommands {
  register(id: string, handler: Function): () => void;
  registerDynamic(id: string, handler: Function): IDynamicShellCommand;
  invoke<T extends any[]>(id: string, ...args: T): unknown;
  invokeWithArgs<T extends any[]>(id: string, args?: T | undefined): unknown;
}

export const ShellCommands = new class ShellCommands implements IShellCommands {
  private registry: Record<IShellCommandId, Function> = {};

  register(id: IShellCommandId, handler: Function): () => void {
    this.registry[id] = handler;

    return () => delete this.registry[id];
  }

  registerDynamic(id: IShellCommandId, handler: Function): IDynamicShellCommand {
    const actualId = Symbol(id.toString());

    return {
      id: actualId,
      dispose: this.register(actualId, handler),
    };
  }

  invoke<T extends any[]>(id: IShellCommandId, ...args: T) {
    const handler = this.registry[id];
    if (handler) {
      try {
        return handler(...args);
      } catch (e) {
        NotificationState.error(`Command "${id.toString()}" failed:\n${e}`, {
          expireAt: getExpireAt(5000),
        });
      }
    }
  }

  invokeWithArgs<T extends any[]>(id: IShellCommandId, args?: T | undefined) {
    if (args) {
      return this.invoke(id, ...args);
    }

    return this.invoke(id);
  }
}();
