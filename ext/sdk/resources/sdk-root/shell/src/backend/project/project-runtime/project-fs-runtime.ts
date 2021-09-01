import { ApiClient } from "backend/api/api-client";
import { FsMapping } from "backend/fs/fs-mapping";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { wrapWithWhenTickEnds } from "backend/ticker";
import { inject, injectable } from "inversify";
import { projectApi } from "shared/api.events";
import { APIRQ } from "shared/api.requests";
import { FilesystemEntry, FilesystemEntryMap } from "shared/api.types";
import { isAssetMetaFile, stripAssetMetaExt } from "utils/project";
import { ProjectRuntime } from "./project-runtime";

@injectable()
export class ProjectFsRuntime {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsMapping)
  private readonly fsMapping: FsMapping;

  protected rt: ProjectRuntime;

  get(): FilesystemEntryMap {
    return this.fsMapping.getMap();
  }

  async init(rt: ProjectRuntime) {
    this.rt = rt;

    this.fsMapping.setShouldProcessUpdate((path) => path !== this.rt.state.manifestPath);
    this.fsMapping.setEntryMetaExtras(this.rt.getEntryMetaContributions());
    this.fsMapping.setProcessEntry(this.processFsEntry);

    this.fsMapping.setAfterUpdate((updateType, updatedPath, updatedEntry) => {
      this.sendFsUpdate();

      this.rt.fsUpdate.emit({
        type: updateType,
        path: updatedPath,
        entry: updatedEntry,
      });
    });

    this.fsMapping.setOnDeleted((entryPath) => {
      this.rt.fsEntryDeleted.emit(entryPath);
    });

    this.fsMapping.setOnRenamed((entry, oldPath) => {
      this.rt.fsEntryRenamed.emit({ entry, oldPath });
    });

    await this.fsMapping.init(this.rt.state.path, this.rt.state.storagePath);
  }

  async dispose() {
    return this.fsMapping.deinit();
  }

  forceScan(entryPath: string) {
    this.fsMapping.forceEntryScan(entryPath);
  }

  //#region directories api
  async createDirectory({ directoryName, directoryPath }: APIRQ.ProjectCreateDirectory) {
    const directoryFullPath = this.fsService.joinPath(directoryPath, directoryName);

    await this.fsService.mkdirp(directoryFullPath);
  }
  //#endregion

  //#region files api
  async createFile({ filePath, fileName }: APIRQ.ProjectCreateFile) {
    const fileFullPath = this.fsService.joinPath(filePath, fileName);

    await this.fsService.writeFile(fileFullPath, '');
  }
  //#endregion

  //#region general fs api
  async renameEntry(request: APIRQ.RenameEntry) {
    const newEntryPath = this.fsService.joinPath(this.fsService.dirname(request.entryPath), request.newName);

    return this.fsService.rename(request.entryPath, newEntryPath);
  }

  async moveEntry(request: APIRQ.MoveEntry) {
    const { sourcePath, targetPath } = request;

    const newPath = this.fsService.joinPath(targetPath, this.fsService.basename(sourcePath));

    if (newPath === sourcePath) {
      return;
    }

    await this.fsService.rename(sourcePath, newPath);

    this.rt.fsEntryMoved.emit({
      oldPath: sourcePath,
      newPath,
    });
  }

  async deleteEntry(request: APIRQ.DeleteEntry): Promise<APIRQ.DeleteEntryResponse> {
    const { entryPath, hardDelete } = request;

    if (!(await this.fsService.statSafe(entryPath))) {
      return APIRQ.DeleteEntryResponse.Ok;
    }

    if (hardDelete) {
      await this.fsService.rimraf(entryPath);
    } else {
      try {
        await this.fsService.moveToTrashBin(entryPath);
      } catch (e) {
        this.logService.log(`Failed to recycle fs entry: ${e.toString()}`);
        this.rt.forceFSScan(entryPath);
        return APIRQ.DeleteEntryResponse.FailedToRecycle;
      }
    }

    return APIRQ.DeleteEntryResponse.Ok;
  }

  async copyEntry(request: APIRQ.CopyEntry) {
    const sourceDirName = this.fsService.dirname(request.sourcePath);

    if (sourceDirName === request.targetPath) {
      return;
    }

    this.fsService.copy(request.sourcePath, request.targetPath);
  }

  async copyEntries(request: APIRQ.CopyEntries) {
    if (!request.sourcePaths.length) {
      return;
    }

    await Promise.all(request.sourcePaths.map((sourcePath) => {
      return this.fsService.copy(sourcePath, request.targetPath);
    }));
  }
  //#endregion

  private sendFsUpdate = wrapWithWhenTickEnds(() => {
    if (this.fsMapping.hasUpdates()) {
      this.apiClient.emit(projectApi.fsUpdate, this.fsMapping.flushUpdates());
    }
  });

  private processFsEntry = async (entry: FilesystemEntry) => {
    if (entry.path.startsWith(this.rt.state.storagePath)) {
      return;
    }

    if (isAssetMetaFile(entry.name)) {
      const assetPath = stripAssetMetaExt(entry.path);

      return this.fsMapping.forceEntryScan(assetPath);
    }

    this.rt.fsEntry.emit(entry);
  }
}
