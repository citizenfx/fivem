import { DEFAULT_ENABLED_ASSETS } from 'assets/core/contants';
import { ExplorerService } from 'backend/explorer/explorer-service';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { Task } from 'backend/task/task-reporter-service';
import { injectable, inject } from 'inversify';
import { serverUpdateChannels } from 'shared/api.types';
import { AssetMeta, assetMetaFileExt } from 'shared/asset.types';
import { ProjectManifest } from 'shared/project.types';
import { omit } from 'utils/omit';
import { endsWith } from 'utils/stringUtils';

export interface ProjectUpgradeRequest {
  task: Task,
  projectPath: string,
  manifestPath: string,
  storagePath: string,
}

@injectable()
export class ProjectUpgrade {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ExplorerService)
  protected readonly explorerService: ExplorerService;

  private projectFileRestored = false;

  async maybeUpgradeProject(request: ProjectUpgradeRequest) {
    await this.maybeUpgrateAssetMetas(request);

    await this.maybeRestoreProjectFile(request);

    if (!this.projectFileRestored) {
      await this.maybeUpgradeManifestResourcesToAssets(request);
    }
  }

  private async maybeRestoreProjectFile(request: ProjectUpgradeRequest) {
    try {
      await this.fsService.readFileJson(request.manifestPath);
    } catch (e) {
      this.projectFileRestored = true;

      const stat = await this.fsService.statSafe(request.manifestPath);
      const defaultDate = new Date().toISOString();

      let assetRelativePaths: string[] = DEFAULT_ENABLED_ASSETS;

      if (await this.fsService.statSafe(this.fsService.joinPath(request.projectPath, 'cfx-server-data'))) {
        assetRelativePaths = assetRelativePaths.map((assetRelativePath) => {
          return assetRelativePath.replace('system-resources', 'cfx-server-data');
        });
      }

      const manifest: ProjectManifest = {
        createdAt: stat?.birthtime.toISOString() || defaultDate,
        updatedAt: stat?.mtime.toISOString() || defaultDate,
        name: this.fsService.basename(request.projectPath),
        serverUpdateChannel: serverUpdateChannels.latest,
        pathsState: {},
        assets: assetRelativePaths.reduce((acc, assetRelativePath) => {
          acc[assetRelativePath] = {
            enabled: true,
            restartOnChange: false,
          };

          return acc;
        }, {}),
      };

      await this.fsService.writeFileJson(request.manifestPath, manifest, true);
    }
  }

  private async maybeUpgrateAssetMetas(request: ProjectUpgradeRequest) {
    const shadowRootPath = this.fsService.joinPath(request.storagePath, 'shadowRoot');

    this.logService.log('Upgrading asset meta at', shadowRootPath);

    // If no shadow root path - yay, no need to upgrade metas
    if (!(await this.fsService.statSafe(shadowRootPath))) {
      this.logService.log('Assets metas already upgraded');
      return;
    }

    request.task.setText('Upgrading project meta files...');

    const paths = await this.explorerService.readDirRecursively(shadowRootPath);

    const entriesToTraverse = new Set(paths[shadowRootPath]);

    for (const entry of entriesToTraverse) {
      if (entry.isDirectory) {
        const entryChildren = paths[entry.path] || [];
        entryChildren.map((childEntry) => entriesToTraverse.add(childEntry));
        continue;
      }

      if (entry.name === 'fxasset.json') {
        const assetRelativePath = this.fsService.relativePath(shadowRootPath, this.fsService.dirname(entry.path));
        const assetMetaPath = this.fsService.joinPath(request.projectPath, assetRelativePath + assetMetaFileExt);

        const oldAssetMeta: Record<string, any> = await this.fsService.readFileJson(entry.path);

        if (oldAssetMeta.flags) {
          const newAssetMeta: AssetMeta = {
            flags: oldAssetMeta.flags,
          };

          await this.fsService.writeFileJson(assetMetaPath, newAssetMeta);
        }
      }
    }

    await this.fsService.rimraf(shadowRootPath);
  }

  private async maybeUpgradeManifestResourcesToAssets(request: ProjectUpgradeRequest) {
    const manifest: any = await this.fsService.readFileJson(request.manifestPath);

    if (!manifest.resources) {
      return;
    }

    manifest.assets = {};

    const resourceNames = new Set(Object.keys(manifest.resources || []));

    if (resourceNames.size > 0) {
      const folderPaths = Object
        .keys(await this.explorerService.readDirRecursively(request.projectPath))
        .filter((folderPath) => folderPath !== request.projectPath && (folderPath.indexOf('.fxdk') === -1))
        .sort((a, b) => a.length - b.length);

      for (const resourceName of resourceNames) {
        const resourcePath = folderPaths.find((folderPath) => endsWith(folderPath, resourceName));

        if (resourcePath) {
          const assetRelativePath = this.fsService.relativePath(request.projectPath, resourcePath);

          manifest.assets[assetRelativePath] = {
            enabled: false,
            ...omit(manifest.resources[resourceName] || {}, 'name'),
          };

          continue;
        }
      }
    }

    delete manifest.resources;

    await this.fsService.writeFileJson(request.manifestPath, manifest, true);
  }
}
