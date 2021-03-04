import { ExplorerService } from 'backend/explorer/explorer-service';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { Task } from 'backend/task/task-reporter-service';
import { injectable, inject } from 'inversify';
import { AssetMeta, assetMetaFileExt } from 'shared/asset.types';

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

  async maybeUpgradeProject(request: ProjectUpgradeRequest) {
    await this.maybeUpgrateAssetMetas(request);
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
}
