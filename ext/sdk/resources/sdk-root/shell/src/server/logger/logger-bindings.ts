import { interfaces } from "inversify";
import { bindContributionProvider } from "server/contribution-provider";
import { ConsoleLogger } from "./console-logger";
import { bindLogProvider, LogProvider } from "./log-provider";
import { LogService } from "./log-service";

export const bindLogger = (container: interfaces.Container) => {
  container.bind(LogService).toSelf().inSingletonScope();

  bindContributionProvider(container, LogProvider);

  container.bind(ConsoleLogger).toSelf().inSingletonScope();
  bindLogProvider(container, ConsoleLogger);
};
