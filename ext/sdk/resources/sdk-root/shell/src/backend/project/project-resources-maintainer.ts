import fs from 'fs';
import { injectable, inject } from 'inversify';
import { FeaturesService } from 'backend/features/features-service';
import { GameServerService } from 'backend/game-server/game-server-service';
import { LogService } from 'backend/logger/log-service';
import { FsService } from 'backend/fs/fs-service';
import { Feature, ServerStates } from 'shared/api.types';
import { ConfigService } from 'backend/config-service';
import { debounce } from 'shared/utils';
import { ProjectAccess } from './project-access';

enum ResourceReconcilationState {
  start = 1,
  stop,
  idle,
}

@injectable()
export class ProjectResourcesMaintainer {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(FeaturesService)
  protected readonly featuresService: FeaturesService;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @inject(GameServerService)
  protected readonly gameServerService: GameServerService;

  protected currentEnabledResourcesPaths = new Set<string>();

  refreshEnabledResources() {
    this.gameServerService.lock();

    this.doRefreshEnabledResources();
  }

  private doRefreshEnabledResources = debounce(async () => {
    const enabledResourcesPaths = this.projectAccess.getInstance().getEnabledResourcesPaths();
    const fxserverCwd = this.projectAccess.getInstance().getFxserverCwd();

    await this.linkResources(fxserverCwd, enabledResourcesPaths);

    this.gameServerService.unlock();

    this.reconcileEnabledResourcesAndRefresh(enabledResourcesPaths);
  }, 5);

  private async linkResource(source: string, dest: string) {
    const windowsDevModeEnabled = await this.featuresService.get(Feature.windowsDevModeEnabled);

    const linkType = windowsDevModeEnabled
      ? 'dir'
      : 'junction';

    return fs.promises.symlink(source, dest, linkType);
  }

  private async linkResources(fxserverCwd: string, resourcesPaths: string[]) {
    const resourcesDirectoryPath = this.fsService.joinPath(fxserverCwd, 'resources');

    await this.fsService.rimraf(resourcesDirectoryPath);
    await this.fsService.mkdirp(resourcesDirectoryPath);

    const links = resourcesPaths.map((resourcePath) => ({
      source: resourcePath,
      dest: this.fsService.joinPath(resourcesDirectoryPath, this.fsService.basename(resourcePath)),
    }));

    links.unshift({
      source: this.configService.sdkGame,
      dest: this.fsService.joinPath(resourcesDirectoryPath, 'sdk-game'),
    });

    await Promise.all(
      links.map(async ({ source, dest }) => {
        if (await this.fsService.statSafe(dest)) {
          const destRealpath = await fs.promises.realpath(dest);

          if (destRealpath !== source) {
            await fs.promises.unlink(dest);
          }
        }

        try {
          await this.linkResource(source, dest);
        } catch (e) {
          this.logService.log('Failed to link resource', e.toString());
        }
      }),
    );
  }

  /**
   * Starts newly enabled resources,
   * Stops disabled resources
   *
   * Asks server to refresh it's state
   */
  private reconcileEnabledResourcesAndRefresh(enabledResourcesPaths: string[]) {
    if (this.gameServerService.getState() !== ServerStates.up) {
      this.currentEnabledResourcesPaths = new Set(enabledResourcesPaths);
      return;
    }

    this.logService.log('Reconciling enabled resources', { enabledResourcesPaths });

    this.gameServerService.emitSdkGameEvent('refresh');

    const resourcesStates = {};

    enabledResourcesPaths.forEach((resourcePath) => {
      const resourceName = this.fsService.basename(resourcePath);

      resourcesStates[resourceName] = this.currentEnabledResourcesPaths.has(resourcePath)
        ? ResourceReconcilationState.idle
        : ResourceReconcilationState.start;
    });

    this.currentEnabledResourcesPaths.forEach((resourcePath) => {
      const resourceName = this.fsService.basename(resourcePath);

      if (!resourcesStates[resourceName]) {
        resourcesStates[resourceName] = ResourceReconcilationState.stop;
      }
    });

    Object.entries(resourcesStates).forEach(([resourceName, state]) => {
      if (state === ResourceReconcilationState.start) {
        return this.gameServerService.emitSdkGameEvent('start', resourceName);
      }

      if (state === ResourceReconcilationState.stop) {
        return this.gameServerService.emitSdkGameEvent('stop', resourceName);
      }
    });

    this.currentEnabledResourcesPaths = new Set(enabledResourcesPaths);
  }
}
