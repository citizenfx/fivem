import { injectable, inject } from 'inversify';
import { AppContribution } from 'backend/app/app-contribution';
import { ConfigService } from 'backend/config-service';
import { Deferred } from 'backend/deferred';
import { FsService } from 'backend/fs/fs-service';
import { ServerResourceDescriptor } from 'backend/game-server/game-server-runtime';
import { GitService } from 'backend/git/git-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { SystemResource, SYSTEM_RESOURCES_DEPENDENCIES, SYSTEM_RESOURCES_MAPPING, SYSTEM_RESOURCES_URL } from './system-resources-constants';

function getNormalizedDeps(resourceName: SystemResource): SystemResource[] {
  if (!SYSTEM_RESOURCES_DEPENDENCIES[resourceName]) {
    return [];
  }

  const deps = SYSTEM_RESOURCES_DEPENDENCIES[resourceName].slice();

  for (const dep of SYSTEM_RESOURCES_DEPENDENCIES[resourceName]) {
    deps.push(...getNormalizedDeps(dep));
  }

  return deps;
}

const normalizedDependencies = Object.keys(SYSTEM_RESOURCES_DEPENDENCIES).reduce((acc, resourceName: SystemResource) => {
  acc[resourceName] = getNormalizedDeps(resourceName);

  return acc;
}, {});

const resolvedResourceDescriptorsCache: Record<string, ServerResourceDescriptor[]> = {};

@injectable()
export class SystemResourcesService implements AppContribution {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(GitService)
  protected readonly gitService: GitService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  private available = new Deferred<boolean>();

  boot() {
    this.maybeUpdateSystemResources();
  }

  getAvailablePromise(): Promise<boolean> {
    return this.available.promise;
  }

  getResourcePath(resourceName: SystemResource): string {
    return this.fsService.joinPath(this.configService.systemResourcesPath, SYSTEM_RESOURCES_MAPPING[resourceName] || '__INVALID_SYSTEM_RESOURCE_NAME__');
  }

  getResourceDescriptors(resourceNames: SystemResource[]): ServerResourceDescriptor[] {
    const cacheKey = JSON.stringify(resourceNames);

    if (!resolvedResourceDescriptorsCache[cacheKey]) {
      const resourceNamesSet = new Set(
        resourceNames
          .filter((resourceName) => SYSTEM_RESOURCES_MAPPING[resourceName])
          .reduce((acc, resourceName) => {
            if (!normalizedDependencies[resourceName]) {
              acc.push(resourceName);

              return acc;
            }

            return acc.concat(normalizedDependencies[resourceName]);
          }, []),
      );

      const resourceDescriptors: ServerResourceDescriptor[] = Array(resourceNamesSet.size);
      let iter = 0;

      for (const resourceName of resourceNamesSet) {
        resourceDescriptors[iter++] = {
          name: resourceName,
          path: this.getResourcePath(resourceName),
        };
      }

      resolvedResourceDescriptorsCache[cacheKey] = resourceDescriptors;
    }

    return resolvedResourceDescriptorsCache[cacheKey];
  }

  private async maybeUpdateSystemResources() {
    if ((!await this.fsService.statSafe(this.configService.systemResourcesRoot))) {
      await this.taskReporterService.wrap('Installing system resources', () => this.cloneSystemResources());
    } else {
      await this.taskReporterService.wrap('Updating system resources', () => this.updateSystemResource());
    }
  }

  private async updateSystemResource() {
    try {
      const [remoteHash, localHash] = await Promise.all([
        this.gitService.getRemoteBranchRef(this.configService.systemResourcesRoot, 'origin', 'master'),
        this.gitService.getLocalBranchRef(this.configService.systemResourcesRoot, 'master'),
      ]);

      if (remoteHash !== localHash) {
        await this.gitService.fastForwrad(this.configService.systemResourcesRoot);
      }
    } catch (e) {
      this.logService.error(e);
      this.notificationService.error(`Failed to update system resources: ${e.toString()}`);
    } finally {
      this.available.resolve(true);
    }
  }

  private async cloneSystemResources() {
    try {
      await this.gitService.clone(this.configService.systemResourcesRoot, SYSTEM_RESOURCES_URL);

      this.available.resolve(true);
    } catch (e) {
      this.logService.error(e);
      this.notificationService.error(`Failed to install system resources: ${e.toString()}`);

      this.available.resolve(false);

      await this.fsService.rimraf(this.configService.systemResourcesRoot);
    }
  }
}
