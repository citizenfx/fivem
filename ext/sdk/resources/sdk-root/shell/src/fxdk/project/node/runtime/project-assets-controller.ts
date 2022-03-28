import { AssetRuntime, IAssetRuntime } from "fxdk/project/node/asset/asset-runtime";
import { FsService } from "backend/fs/fs-service";
import { ScopedLogService } from "backend/logger/scoped-logger";
import { dispose, Disposer, IDisposableObject } from "fxdk/base/disposable";
import { AsyncTree, ITreeLeaf, ITreeLeafTraverser } from "fxdk/base/tree";
import { ProjectEvents, ProjectFsEvents } from "../project-events";
import { IFsEntry } from "../../common/project.types";
import { ProjectFsExtensions } from "../projectExtensions";
import { ProjectRuntime } from "./project-runtime";

export class ProjectAssetsController implements IDisposableObject {
  protected readonly logService = new ScopedLogService('ProjectAssets');

  private toDispose = new Disposer();

  private assetsTree: AssetsTree;

  private rt!: ProjectRuntime;

  private hydrated = false;

  constructor() {
    this.toDispose.register(ProjectFsEvents.Hydrated.addListener(this.handleHydrated));

    this.toDispose.register(ProjectEvents.AssetConfigChanged.addListener(this.handleAssetConfigChanged));

    this.toDispose.register(ProjectFsEvents.FsEntrySpawned.addListener(this.handleFsEntrySpawned));
    this.toDispose.register(ProjectFsEvents.FsEntryUpdated.addListener(this.handleFsEntryUpdated));

    this.toDispose.register(ProjectFsEvents.AfterDeleted.addListener(this.handleAfterDeleted));

    this.toDispose.register(ProjectFsEvents.BeforeRename.addListener(this.handleBeforeRenamed));
    this.toDispose.register(ProjectFsEvents.AfterRenamed.addListener(this.handleAfterRenamed));
  }

  async init(rt: ProjectRuntime) {
    this.rt = rt;

    this.assetsTree = this.toDispose.register(new AssetsTree(rt.state.path));
  }

  async dispose() {
    dispose(this.toDispose);
  }

  has(assetPath: string): boolean {
    return !!this.assetsTree.get(assetPath);
  }

  get(assetPath: string): IAssetRuntime | undefined {
    return this.assetsTree.get(assetPath);
  }

  getAllPaths(): string[] {
    return this.assetsTree.getAllPaths();
  }

  private spawnAsset(entry: IFsEntry, entryPath: string): AssetRuntime | undefined {
    if (entry.handle === 'file' || entry.handle === 'directory') {
      return;
    }

    const handler = ProjectFsExtensions.getHandler(entry);
    if (!handler) {
      return;
    }

    if (!handler.spawnAssetRuntime) {
      return;
    }

    const asset = handler.spawnAssetRuntime(entry, entryPath);

    this.assetsTree.add(entryPath, asset);

    if (this.hydrated) {
      asset?.init();
    }

    ProjectEvents.AssetSpawned.emit(asset);

    return asset;
  }

  private readonly handleHydrated = () => {
    this.hydrated = true;

    this.assetsTree.getAll().forEach((asset) => asset.init());
  };

  private readonly handleAssetConfigChanged = (event: ProjectEvents.AssetConfigChangedEvent) => {
    return this.assetsTree.get(event.assetPath)?.handleConfigChanged?.(event.config);
  };

  private readonly handleFsEntrySpawned = (event: ProjectFsEvents.FsEntrySpawnedEvent) => {
    const { fsEntry, entryPath } = event;

    const asset = this.assetsTree.get(entryPath);
    if (!asset) {
      this.spawnAsset(fsEntry, entryPath);
    }

    return this.invokeOnParentAssets(entryPath, 'acceptNestedFsEntrySpawned', event);
  };

  private readonly handleFsEntryUpdated = async (event: ProjectFsEvents.FsEntryUpdatedEvent) => {
    const { fsEntry, entryPath } = event;

    let asset = this.assetsTree.get(entryPath);

    // No asset yet- may be spawn it
    if (!asset) {
      this.spawnAsset(fsEntry, entryPath);
    } else {
      // If handle for asset at this path changed - kill old one and may be spawn new one
      if (asset.handle !== fsEntry.handle) {
        await this.assetsTree.delete(entryPath);
        this.spawnAsset(fsEntry, entryPath);
      }
    }

    return this.invokeOnParentAssets(entryPath, 'acceptNestedFsEntryUpdated', event);
  };

  private readonly handleBeforeRenamed = async (event: ProjectFsEvents.BeforeRenamedEvent) => {
    await this.assetsTree.deleteDeep(event.oldEntryPath);
  };

  private readonly handleAfterRenamed = async (event: ProjectFsEvents.AfterRenamedEvent) => {
    this.assetsTree.relocate(event.oldEntryPath, event.entryPath);

    await this.assetsTree.traverseOptionalItemsWithin(event.entryPath, async (asset, fullAssetPath) => {
      const fsEntry = this.rt.fs.getFsEntry(fullAssetPath);
      if (!fsEntry) {
        return this.assetsTree.delete(fullAssetPath);
      }

      if (asset) {
        await this.assetsTree.delete(fullAssetPath);
      }

      this.spawnAsset(fsEntry, fullAssetPath);
    });

    return this.invokeOnParentAssets(event.entryPath, 'acceptNestedAfterRenamed', event);
  };

  private readonly handleAfterDeleted = async (event: ProjectFsEvents.AfterDeletedEvent) => {
    // Delete asset and all it's children
    this.assetsTree.prune(event.oldEntryPath);

    return this.invokeOnParentAssets(event.oldEntryPath, 'acceptNestedAfterDeleted', event);
  };

  private invokeOnParentAssets<M extends keyof IAssetRuntime, A extends Parameters<Extract<IAssetRuntime[M], Function>>>(
    assetPath: string,
    method: A extends any[] ? M : never,
    ...args: A
  ) {
    return this.assetsTree.traversePath(assetPath, (leaf) => {
      const asset = leaf.item;
      if (!asset) {
        return;
      }

      const methodFn = asset[method];

      if (typeof methodFn === 'function') {
        return methodFn.call(asset, ...args);
      }
    });
  }
}

export type IAssetTreeLeaf = ITreeLeaf<IAssetRuntime>;
export type IAssetTreeTraverser = ITreeLeafTraverser<IAssetRuntime>;

class AssetsTree extends AsyncTree<IAssetRuntime> {
  constructor(rootPath: string) {
    super(rootPath, FsService.separator);
  }

  getAllPaths(): string[] {
    return Object.keys(this.pathsMap);
  }

  protected override async deleteLeafItem(leaf: IAssetTreeLeaf, fullEntryPath: string) {
    await dispose(leaf.item);

    super.deleteLeafItem(leaf, fullEntryPath);
  }

  protected override itemToJSON(asset: IAssetRuntime) {
    return {
      name: asset.getName(),
      path: asset.fsEntryPath,
      resourceDescriptor: asset.getResourceDescriptor?.(),
    };
  }
}
