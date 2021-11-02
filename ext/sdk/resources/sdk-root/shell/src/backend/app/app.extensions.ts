import { getContainer } from "backend/container-access";
import { interfaces } from "inversify";

export const AppContribution = Symbol('AppContribution');
export interface AppContribution {
  boot?(): any,

  prepare?(): any,

  beforeAppStart?(): any,
  afterAppStart?(): any,
}

export const registerAppContribution = <T>(service: interfaces.ServiceIdentifier<T>) => {
  getContainer().bind(AppContribution).toService(service);

  return service;
};
