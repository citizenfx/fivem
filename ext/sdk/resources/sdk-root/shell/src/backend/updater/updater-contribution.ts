import { getContainer } from "backend/container-access";
import { interfaces } from "inversify";

export const UpdaterContribution = Symbol('UpdaterContribution');
export interface UpdaterContribution {
  update(): Promise<void>;
}

export const registerUpdaterContribution = <T>(serviceId: interfaces.ServiceIdentifier<T>) => {
  getContainer().bind(UpdaterContribution).toService(serviceId);

  return serviceId;
};
