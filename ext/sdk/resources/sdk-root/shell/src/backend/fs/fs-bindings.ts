import { interfaces } from "inversify";
import { FsService } from "./fs-service";

export const bindFs = (container: interfaces.Container) => {
  container.bind(FsService).toSelf().inSingletonScope();
};
