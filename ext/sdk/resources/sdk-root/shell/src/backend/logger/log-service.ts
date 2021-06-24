import { inject, injectable, named } from "inversify";
import { ContributionProvider } from "backend/contribution-provider";
import { LogProvider } from "./log-provider";

@injectable()
export class LogService implements LogProvider {
  @inject(ContributionProvider) @named(LogProvider)
  protected readonly loggersProvider: ContributionProvider<LogProvider>;

  setUserId(id: string) {
    this.loggersProvider.getAll().forEach((logger) => logger.setUserId?.(id));
  }

  log<T extends any[]>(...args: T) {
    this.loggersProvider.getAll().forEach((logger) => logger.log?.(...args));
  }

  error<T extends Error>(error: T, extra?: Record<string, any>) {
    this.loggersProvider.getAll().forEach((logger) => logger.error?.(error, extra));
  }
}
