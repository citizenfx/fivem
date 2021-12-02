import { lazyInject } from "backend/container-access";
import { LogService } from "./log-service";

const scopeSymbol = Symbol('Scope');

export class ScopedLogService {
  @lazyInject(LogService)
  protected readonly logService: LogService;

  constructor(private readonly scope: string) {}

  log(message: string, ...args: any[]) {
    this.logService.log(`[${this.scope}] ${message}`, ...args);
  }

  error<T extends Error>(error: T, extra: Record<string, any> | null = null) {
    this.logService.error(error, { [scopeSymbol]: this.scope, ...extra });
  }
}
