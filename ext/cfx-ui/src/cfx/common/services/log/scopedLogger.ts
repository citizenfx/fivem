import { inject, named } from 'inversify';

import { LogService } from './logService';

/**
 * !DECORATOR
 */
export const scopedLogger = (name: string): ReturnType<typeof inject> => (...args) => {
  // @ts-expect-error
  inject(ScopedLogger)(...args);
  named(name)(...args);
};

export class ScopedLogger {
  private prefix = '';

  constructor(
    protected readonly logService: LogService,
    private name: string,
  ) {
    this.prefix = `[${name}]`;
  }

  log<T extends any[]>(...args: T) {
    this.logService.log(this.prefix, ...args);
  }

  error<T extends Error>(error: T, extra: Record<string, any> = {}) {
    this.logService.error(error, { ...extra,
      loggerName: this.name });
  }
}
