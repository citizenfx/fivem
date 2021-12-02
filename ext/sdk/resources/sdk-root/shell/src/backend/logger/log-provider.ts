import { getContainer } from "backend/container-access";
import { interfaces } from "inversify";

export const LogProvider = Symbol('LogProvider');
export interface LogProvider {
  setUserId?(id: string): void;

  log?<T extends any[]>(...args: T): void;

  error?<T extends Error>(error: T, extra?: Record<string, any>): void;
}

export function registerLogProvider<T>(service: interfaces.ServiceIdentifier<T>) {
  getContainer().bind(LogProvider).toService(service);
};
