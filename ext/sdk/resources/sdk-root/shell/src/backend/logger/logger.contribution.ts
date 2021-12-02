import { registerContributionProvider } from "backend/contribution-provider";
import { ConsoleLogger } from "./console-logger";
import { registerLogProvider, LogProvider } from "./log-provider";
import { LogService } from "./log-service";
import { SentryLogger } from "./sentry-logger";
import { registerSingleton } from "backend/container-access";

registerSingleton(LogService);

registerContributionProvider(LogProvider);

registerLogProvider(
  registerSingleton(ConsoleLogger)
);

registerLogProvider(
  registerSingleton(SentryLogger)
);
