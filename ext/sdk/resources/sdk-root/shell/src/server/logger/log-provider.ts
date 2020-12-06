import { interfaces } from "inversify";

export const LogProvider = Symbol('LogProvider');
export interface LogProvider {
  log<T extends any[]>(...args: T): void;
}

export const bindLogProvider = <T>(container: interfaces.Container, service: interfaces.ServiceIdentifier<T>) => {
  container.bind(LogProvider).toService(service);
};
