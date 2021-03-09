import { interfaces } from "inversify";

export const LogProvider = Symbol('LogProvider');
export interface LogProvider {
  setUserId?(id: string): void;

  log?<T extends any[]>(...args: T): void;

  error?<T extends Error>(error: T, extra?: Record<string, any>): void;
}

export const bindLogProvider = <T>(container: interfaces.Container, service: interfaces.ServiceIdentifier<T>) => {
  container.bind(LogProvider).toService(service);
};
