import { interfaces } from "inversify";

export const AppServiceFactory = Symbol('AppServiceFactory');
export type AppServiceFactory = <T>(service: interfaces.Newable<T>) => T;
