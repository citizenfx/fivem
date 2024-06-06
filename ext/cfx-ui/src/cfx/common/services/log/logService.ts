import { injectable, interfaces, multiInject, optional } from 'inversify';

import { LogProvider } from './logService.extensions';
import { ScopedLogger } from './scopedLogger';
import { ServicesContainer } from '../../../base/servicesContainer';

@injectable()
export class LogService {
  @multiInject(LogProvider)
  @optional()
  protected readonly loggersProvider: LogProvider[];

  setUserId(id: string) {
    this.loggersProvider.forEach((logger) => logger.setUserId?.(id));
  }

  log<T extends any[]>(...args: T) {
    this.loggersProvider.forEach((logger) => logger.log?.(...args));
  }

  error<T extends Error>(error: T, extra?: Record<string, any>) {
    this.loggersProvider.forEach((logger) => logger.error?.(error, extra));
  }
}

export function registerLogService(container: ServicesContainer, providers: interfaces.Newable<LogProvider>[] = []) {
  container.register(LogService);

  providers.forEach((provider) => {
    container.registerImpl(LogProvider, provider);
  });

  container.registerDynamic(ScopedLogger, (ctx: interfaces.Context) => {
    const loggerName = ctx.currentRequest.target.getNamedTag();

    if (loggerName === null) {
      throw new Error('ScopedLogger must be injected with @named(\'LoggerName\')');
    }

    const logService = ctx.container.get(LogService);

    return new ScopedLogger(logService, loggerName.value);
  });
}
