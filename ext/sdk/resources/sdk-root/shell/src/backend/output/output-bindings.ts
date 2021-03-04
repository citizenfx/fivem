import { interfaces } from "inversify";
import { OutputService } from "./output-service";

export const bindOutput = (container: interfaces.Container) => {
  container.bind(OutputService).toSelf().inSingletonScope();
};
