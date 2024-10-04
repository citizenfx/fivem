import { lazyInject } from "backend/container-access";
import { Deferred } from "backend/deferred";
import { ScopedLogService } from "backend/logger/scoped-logger";
import { NotificationService } from "backend/notification/notification-service";
import { injectable } from "inversify";
import { ProjectInstanceService } from "./project";

export type ProjectUsageReason =
  | string
  | { silent: true, reason: string };

export function SilentUsageReason(reason: string): { silent: true, reason: string } {
  return {
    silent: true,
    reason,
  };
}

@injectable()
export class ProjectAccess {
  @lazyInject(NotificationService)
  protected readonly notificationService: NotificationService;

  protected readonly logService = new ScopedLogService('ProjectAccess');

  private project: ProjectInstanceService | null = null;

  private usageCounter = 0;
  private noUsageDeferreds: Set<Deferred<void>> = new Set();

  setInstance(project: ProjectInstanceService | null) {
    this.project = project;
  }

  hasInstance(): boolean {
    return this.project !== null;
  }

  getInstance(): ProjectInstanceService {
    if (!this.hasInstance()) {
      throw new Error('No project is open');
    }

    return this.project!;
  }

  getNoUsagePromise(): Promise<void> {
    if (this.usageCounter === 0) {
      return Promise.resolve();
    }

    const deferred = new Deferred();

    this.noUsageDeferreds.add(deferred);

    return deferred.promise;
  }

  async useInstance<T>(usageReason: ProjectUsageReason, cb: (project: ProjectInstanceService) => Promise<T> | T): Promise<T | void> {
    if (!this.hasInstance()) {
      if (typeof usageReason === 'string') {
        this.notificationService.error(`No project is open, failed to ${usageReason}`);
      } else {
        this.logService.error(new Error(`No project is open, failed to ${usageReason}`));
      }
      return;
    }

    this.increaseUsage();

    try {
      return await cb(this.project!);
    } catch (e) {
      if (typeof usageReason === 'string') {
        this.notificationService.error(`Failed to ${usageReason}: ${e.toString()}`);
      } else {
        this.logService.error(e);
      }
      throw e;
    } finally {
      this.decreaseUsage();
    }
  }

  private increaseUsage() {
    this.usageCounter++;
  }

  private decreaseUsage() {
    this.usageCounter--;

    if (this.usageCounter === 0) {
      const deferreds = this.noUsageDeferreds;
      this.noUsageDeferreds = new Set();

      for (const deferred of deferreds) {
        deferred.resolve();
      }
    }
  }
}
