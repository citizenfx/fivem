import { interfaces } from "inversify";
import { ServicesContainer } from "../../../base/servicesContainer";

export const LogProvider = Symbol('LogProvider');
export interface LogProvider {
  setUserId?(id: string): void;

  log?<T extends any[]>(...args: T): void;

  error?<T extends Error>(error: T, extra?: Record<string, any>): void;
}

export function registerLogProvider(container: ServicesContainer, service: interfaces.Newable<LogProvider>) {
  container.registerImpl(LogProvider, service);
};
