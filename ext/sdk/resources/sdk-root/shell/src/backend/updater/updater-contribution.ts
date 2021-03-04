import { interfaces } from "inversify";

export const UpdaterContribution = Symbol('UpdaterContribution');
export interface UpdaterContribution {
  update(): Promise<void>;
}

export const bindUpdaterContribution = <T>(container: interfaces.Container, serviceId: interfaces.ServiceIdentifier<T>) => {
  container.bind(UpdaterContribution).toService(serviceId);
};
