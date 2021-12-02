import { resourceManifestLegacyFilename, resourceManifestFilename } from "backend/constants";
import { lazyInject } from "backend/container-access";
import { FsService } from "backend/fs/fs-service";
import { FsWatcherEvent, FsWatcherEventType, createFsWatcherEvent } from "backend/fs/fs-watcher";
import { IFsEntryHandler } from "fxdk/project/node/projectExtensions";
import { IFsEntry } from "fxdk/project/common/project.types";
import { ResourceManifestKind } from "../common/resourceManifest";
import { getResourceManifestKind } from "../common/resource.utils";
import { ResourceAssetRuntime } from "./resourceAssetRuntime";

export class ResourceFsEntryHandler implements IFsEntryHandler {
  @lazyInject(FsService)
  protected readonly fsService: FsService;

  async handles(entry: IFsEntry, entryPath: string): Promise<boolean> {
    if (!entry.isDirectory) {
      return false;
    }

    if (entry.childrenScanned) {
      if (entry.children[resourceManifestLegacyFilename]) {
        return true;
      }

      if (entry.children[resourceManifestFilename]) {
        return true;
      }
    }

    const [legacyManifest, moderManifest] = await Promise.all([
      this.fsService.statSafe(this.fsService.joinPath(entryPath, resourceManifestLegacyFilename)),
      this.fsService.statSafe(this.fsService.joinPath(entryPath, resourceManifestFilename)),
    ]);

    return Boolean(legacyManifest) || Boolean(moderManifest);
  }

  spawnAssetRuntime(fsEntry: IFsEntry, fsEntryPath: string) {
    return new ResourceAssetRuntime(fsEntry, fsEntryPath);
  }

  expandEvent([type, entryPath]: FsWatcherEvent) {
    if (getResourceManifestKind(this.fsService.basename(entryPath)) === ResourceManifestKind.none) {
      return;
    }

    switch (type) {
      case FsWatcherEventType.CREATED:
      case FsWatcherEventType.DELETED:
      case FsWatcherEventType.RENAMED: {
        return [
          createFsWatcherEvent(FsWatcherEventType.MODIFIED, this.fsService.dirname(entryPath)),
        ];
      }
    }
  }
}
