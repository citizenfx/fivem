import { SystemResource } from "backend/system-resources/system-resources-constants";
import { dispose, Disposer, IDisposableObject } from "fxdk/base/disposable";
import { Api } from "fxdk/browser/Api";
import { ProjectBuilderCommands } from "fxdk/project/contrib/builder/builder.commands";
import { ExplorerRuntime } from "fxdk/project/contrib/explorer/explorer.runtime";
import { deleteIcon } from "fxdk/ui/icons";
import { makeAutoObservable } from "mobx";
import { FXCodeState } from "personalities/fxcode/FXCodeState";
import { APIRQ } from "shared/api.requests";
import { ServerUpdateChannel } from "shared/api.types";
import { ProjectAssetBaseConfig, ProjectManifest, ProjectOpenData } from "shared/project.types";
import { ShellCommands } from "shell-api/commands";
import { ShellEvents } from "shell-api/events";
import { ConfirmationsState } from "store/ConfirmationsState";
import { ProjectFsState } from "fxdk/project/browser/state/parts/FileSystem";
import { RelocationContext } from "fxdk/project/browser/state/parts/RelocationContext";
import { ProjectLocalStorage } from "./projectLocalStorage";
import { AssetRuntimeDataStore } from "./parts/AssetRuntimeDataStore";
import { ProjectApi } from "fxdk/project/common/project.api";
import { ProjectPathsState, ProjectVariableSetter } from "fxdk/project/common/project.types";

export class ProjectInstance implements IDisposableObject {
  public readonly path: string;

  public readonly pathsState: ProjectPathsState;
  public readonly fileSystem: ProjectFsState;
  public readonly relocation: RelocationContext;
  public readonly localStorage: ProjectLocalStorage;
  public readonly assetRuntimeData: AssetRuntimeDataStore;

  public selectedEntryPath = '';

  get manifest(): ProjectManifest {
    return this._manifest;
  }

  get name(): string {
    return this.manifest.name;
  }

  private toDispose: Disposer;
  private _manifest: ProjectManifest;

  constructor({ rootFsEntry, path, manifest, pathsState }: ProjectOpenData) {
    this.path = path;
    this.pathsState = pathsState;
    this._manifest = manifest;

    // That is so when used in project settings - elements inside can be normally tracked by mobx
    this.manifest.variables ??= {};

    makeAutoObservable(this);

    this.toDispose = new Disposer();

    this.relocation = new RelocationContext();
    this.fileSystem = this.toDispose.register(new ProjectFsState(this.path, rootFsEntry));
    this.localStorage = this.toDispose.register(new ProjectLocalStorage(this.path));
    this.assetRuntimeData = this.toDispose.register(new AssetRuntimeDataStore());

    this.toDispose.register(ShellEvents.on('fxdk:revealFile', this.revealFile));
    this.toDispose.register(ShellEvents.on('fxdk:buildProject', this.buildProject));

    this.toDispose.register(Api.on(ProjectApi.ManifestEndpoints.update, this.handleManifestUpdate));
  }

  dispose() {
    dispose(this.toDispose);
  }

  getAssetConfig<T extends ProjectAssetBaseConfig>(assetPath: string): Partial<Omit<T, 'enabled'>> & ProjectAssetBaseConfig {
    const config = this.manifest.assets[this.getRelativePath(assetPath)] as any;

    if (!config) {
      return {
        enabled: false,
      } as any;
    }

    return config;
  }

  setAssetConfig<T extends ProjectAssetBaseConfig>(assetPath: string, config: Partial<T>) {
    // Store this naively to faster reflect change
    const relativeAssetPath = this.getRelativePath(assetPath);

    this.manifest.assets[relativeAssetPath] = {
      ...(this.manifest.assets[relativeAssetPath] || null),
      ...config,
    };

    const request: APIRQ.ProjectSetAssetConfig = {
      assetPath,
      config,
    };

    Api.send(ProjectApi.AssetEndpoints.setAssetConfig, request);
  }

  readonly setServerUpdateChannel = (channel: ServerUpdateChannel) => {
    Api.send(ProjectApi.ManifestEndpoints.setServerUpdateChannel, channel);
  };

  toggleSystemResource(resource: SystemResource) {
    const index = this.manifest.systemResources.indexOf(resource);

    if (index !== -1) {
      this.manifest.systemResources.splice(index, 1);
    } else {
      this.manifest.systemResources.push(resource);
    }

    Api.send(ProjectApi.ManifestEndpoints.setSystemResources, this.manifest.systemResources);
  }

  readonly select = (entryPath: string, scrollIntoView = true) => {
    if (this.selectedEntryPath) {
      ExplorerRuntime.getItem(this.selectedEntryPath)?.setSelected(false);
    }

    this.selectedEntryPath = entryPath;

    ExplorerRuntime.getItem(entryPath)?.setSelected(true, scrollIntoView);
  };

  openFile(filePath: string, pinned = false) {
    FXCodeState.openFile(filePath, pinned);

    this.select(filePath);
  }

  findInFiles(entryPath: string) {
    FXCodeState.findInFiles(this.getRelativePath(entryPath));
  }

  getRelativePath(inProjectPath: string): string {
    return inProjectPath.substr(this.path.length + 1);
  }

  shallowScanEntryChildren(entryPath: string) {
    Api.send(ProjectApi.FsEndpoints.shallowScanChildren, entryPath);
  }

  setVariable(variable: string, setter: ProjectVariableSetter, value: string | number | boolean) {
    this.manifest.variables ??= {};

    this.manifest.variables[variable] = {
      setter,
      value,
    };

    Api.send(ProjectApi.ManifestEndpoints.setVariable, {
      variable,
      setter,
      value,
    } as ProjectApi.ManifestRequests.SetVariable);
  }

  deleteVariable(variable: string) {
    if (this.manifest.variables) {
      delete this.manifest.variables[variable];

      Api.send(ProjectApi.ManifestEndpoints.deleteVariable, variable);
    }
  }

  readonly setPathState = (path: string, state: boolean) => {
    this.pathsState[path] = state;

    Api.send(ProjectApi.FsEndpoints.setPathsStatePatch, {
      [path]: state,
    });
  };

  readonly setPathsState = (patch: ProjectPathsState) => {
    Object.assign(this.pathsState, patch);

    Api.send(ProjectApi.FsEndpoints.setPathsStatePatch, patch);
  };

  readonly deleteEntryConfirmFirst = (entryPath: string, title: string, children: () => React.ReactNode) => {
    ConfirmationsState.requestConfirm({
      title,
      buttonIcon: deleteIcon,
      buttonText: 'Delete',
      onConfirm: () => this.deleteEntry(entryPath),
      children,
    });
  };

  readonly deleteEntry = (entryPath: string) => {
    Api.sendCallback(ProjectApi.FsEndpoints.deleteEntry, { entryPath }, (error, response) => {
      if (error) {
        return;
      }

      if (response === APIRQ.DeleteEntryResponse.FailedToRecycle) {
        ConfirmationsState.requestConfirm({
          title: 'Failed to recycle, delete permanently?',
          buttonIcon: deleteIcon,
          buttonText: 'Delete permanently',
          onConfirm() {
            const request: APIRQ.DeleteEntry = {
              entryPath,
              hardDelete: true,
            };

            Api.sendCallback(ProjectApi.FsEndpoints.deleteEntry, request, (error) => {
              if (error) {
                console.error(error);
              }
            });
          },
        });
      }
    });
  };

  readonly revealFile = (entryPathRaw: unknown) => {
    if (typeof entryPathRaw !== 'string') {
      return;
    }

    if (!entryPathRaw.startsWith(this.path.toLowerCase())) {
      return;
    }

    const relativeEntryPath = entryPathRaw.substr(this.path.length + 1);
    if (!relativeEntryPath) {
      return;
    }

    // FXCode will always lower-case drive letter, so we fix it
    const entryPath = `${this.path}\\${relativeEntryPath}`;
    const entryDirPath = entryPath.substr(0, entryPath.lastIndexOf('\\'));
    const entryPathParts = relativeEntryPath.split('\\');

    // Open corresponding paths, if not in project root
    if (entryDirPath !== this.path) {
      let dirPath = this.path;
      const patch: Record<string, boolean> = {};

      while (entryPathParts.length) {
        dirPath += '\\' + entryPathParts.shift();

        patch[dirPath] = true;
      }

      this.setPathsState(patch);
    }

    this.select(entryPath);
  };

  readonly buildProject = (overrides?: Partial<APIRQ.ProjectBuild>) => {
    const buildPath = this.localStorage.buildPath;
    const useVersioning = this.localStorage.buildUseVersioning;
    const deployArtifact = this.localStorage.buildUseArtifact;
    const steamWebApiKey = this.localStorage.steamWebApiKey;
    const tebexSecret = this.localStorage.tebexSecret;

    if (!buildPath) {
      return ShellCommands.invoke(ProjectBuilderCommands.OPEN);
    }

    Api.send(ProjectApi.BuilderEndpoints.build, {
      buildPath,
      useVersioning,
      deployArtifact,
      steamWebApiKey,
      tebexSecret,
      ...overrides,
    } as APIRQ.ProjectBuild);
  };

  private readonly handleManifestUpdate = (manifest: ProjectManifest) => {
    this._manifest = manifest;
  };
}
