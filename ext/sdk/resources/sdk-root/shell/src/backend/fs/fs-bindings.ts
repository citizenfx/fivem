import { interfaces } from "inversify";
import { FsService } from "./fs-service";
import { FsMapping } from "./fs-mapping";

export const bindFs = (container: interfaces.Container) => {
  container.bind(FsService).toSelf().inSingletonScope();

  container.bind(FsMapping).toDynamicValue((ctx) => ctx.container.resolve(FsMapping));
};
