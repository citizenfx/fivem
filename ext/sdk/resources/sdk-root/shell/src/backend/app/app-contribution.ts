import { interfaces } from "inversify";

export const AppContribution = Symbol('AppContribution');
export interface AppContribution {
  boot?(): any,

  prepare?(): any,

  beforeAppStart?(): any,
  afterAppStart?(): any,
}

export const bindAppContribution = <T>(container: interfaces.Container, service: interfaces.ServiceIdentifier<T>) => {
  container.bind(AppContribution).toService(service);
};
