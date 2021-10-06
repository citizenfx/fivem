import React from "react";
import { Api } from "fxdk/browser/Api";
import { APIRQ } from "shared/api.requests";
import { DisposableContainer, DisposableObject } from "backend/disposable-container";
import { makeAutoObservable, reaction } from "mobx";
import { assetApi, projectApi } from "shared/api.events";
import { FilesystemEntry, FilesystemEntryMap } from "shared/api.types";
import { AssetType } from "shared/asset.types";
import { ProjectAssetBaseConfig, ProjectData, ProjectFsUpdate, ProjectManifest, ProjectOpenData, ProjectPathsState, RecentProject } from "shared/project.types";
import { getProjectBuildPathVar, getProjectDeployArtifactVar, getProjectSteamWebApiKeyVar, getProjectTebexSecretVar, getProjectUseVersioningVar } from "utils/projectStorage";
import { ShellState } from "./ShellState";
import { SystemResource } from "backend/system-resources/system-resources-constants";
import { OpenFlag } from "./generic/OpenFlag";
import { ConfirmationsState } from "./ConfirmationsState";
import { deleteIcon } from "constants/icons";
import { AssetDefinition } from "assets/core/asset-interface";
import { FXCodeState } from "personalities/fxcode/FXCodeState";
import { ShellEvents } from "shell-api/events";
import { ShellCommands } from "shell-api/commands";
import { ProjectBuilderCommands } from "fxdk/project/contrib/builder/builder.commands";
import { __DEBUG_MODE_TOGGLES__ } from "constants/debug-constants";
import { ResourceCommands } from "assets/resource/renderer/resource.commands";

export interface FsTreeEntryAccessor {
  setSelected(selected: boolean): void,
}

const fsTreeAccessors: Record<string, FsTreeEntryAccessor> = {};

export function useFsTreeEntryAccessor(entryPath: string, accessorFactory: () => FsTreeEntryAccessor) {
  React.useEffect(() => {
    const accessor = fsTreeAccessors[entryPath] = accessorFactory();

    if (entryPath === ProjectState.project?.selectedEntryPath) {
      accessor.setSelected(true);
    }

    return () => {
      delete fsTreeAccessors[entryPath];
    };
  }, []);
}

export class ProjectObject implements ProjectData, DisposableObject {
  public fs: FilesystemEntryMap;

  public path: string;
  public assets: {
    [path: string]: ProjectAssetBaseConfig,
  };
  public assetDefs: {
    [path: string]: any,
  };
  public manifest: ProjectManifest;

  public assetTypes: Record<string, AssetType | void> = {};

  private disposableContainer: DisposableContainer;

  public selectedEntryPath = '';

  constructor(
    projectData: ProjectData,
    public pathsState: ProjectPathsState,
  ) {
    this.fs = projectData.fs;
    this.path = projectData.path;
    this.assets = projectData.assets;
    this.manifest = projectData.manifest;
    this.assetTypes = projectData.assetTypes;
    this.assetDefs = projectData.assetDefs;

    makeAutoObservable(this);

    this.disposableContainer = new DisposableContainer();

    this.disposableContainer.add(
      Api.on(projectApi.update, this.update),
      Api.on(projectApi.fsUpdate, this.updateFs),
      Api.on(projectApi.pathsStateUpdate, this.updatePathsState),
      Api.on(assetApi.setConfig, this.setAssetConfig),
      Api.on(assetApi.updates, this.handleAssetUpdates),
      ShellEvents.on('fxdk:revealFile', this.revealFile),
    );
  }

  get entry(): FilesystemEntry {
    return {
      path: this.path,
      name: this.path.substr(
        Math.max(
          Math.max(
            this.path.lastIndexOf('/'),
            this.path.lastIndexOf('\\'),
          ) + 1,
          0,
        ),
      ),
      isDirectory: true,
      isFile: false,
      isSymbolicLink: false,
      meta: {
        isFxdkProject: true,
        isResource: false,
        assetMeta: null,
      },
    };
  }

  dispose() {
    this.disposableContainer.dispose();
  }

  getAssetConfig<AssetConfigType extends ProjectAssetBaseConfig>(assetPath: string, defaults: AssetConfigType): AssetConfigType {
    return {
      ...defaults,
      ...this.assets[assetPath],
    };
  }

  toggleSystemResource(resource: SystemResource) {
    const index = this.manifest.systemResources.indexOf(resource);

    if (index !== -1) {
      this.manifest.systemResources.splice(index, 1);
    } else {
      this.manifest.systemResources.push(resource);
    }

    Api.send(projectApi.setSystemResources, this.manifest.systemResources);
  }

  select(entryPath: string) {
    if (this.selectedEntryPath) {
      fsTreeAccessors[this.selectedEntryPath]?.setSelected(false);
    }

    this.selectedEntryPath = entryPath;

    fsTreeAccessors[entryPath]?.setSelected(true);
  }

  readonly setPathState = (path: string, state: boolean) => {
    this.pathsState[path] = state;

    Api.send(projectApi.setPathsStatePatch, {
      [path]: state,
    });
  };

  readonly setPathsState = (patch: Record<string, boolean>) => {
    for (const [key, value] of Object.entries(patch)) {
      this.pathsState[key] = value;
    }

    Api.send(projectApi.setPathsStatePatch, patch);
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
    Api.sendCallback(projectApi.deleteEntry, { entryPath }, (error, response) => {
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

            Api.sendCallback(projectApi.deleteEntry, request, (error) => {
              if (error) {
                console.error(error);
              }
            });
          },
        });
      }
    });
  };

  private update = (projectData: Partial<ProjectData>) => {
    if (projectData.fs) {
      this.fs = projectData.fs;
    }

    if (projectData.manifest) {
      this.manifest = projectData.manifest;
    }
  };

  private updateFs = (update: ProjectFsUpdate) => {
    if (update.replace) {
      Object.entries(update.replace).forEach(([key, value]) => {
        this.fs[key] = value;
      });
    }

    if (update.delete) {
      update.delete.forEach((key) => {
        delete this.fs[key];
      });
    }
  };

  private readonly handleAssetUpdates = (updates: Record<string, { type: AssetType | void, def: AssetDefinition | void }>) => {
    for (const [assetPath, { type, def }] of Object.entries(updates)) {
      this.assetTypes[assetPath] = type;
      this.assetDefs[assetPath] = def;
    }
  };

  private setAssetConfig = ([assetPath, config]) => {
    this.assets[assetPath] = config;
  };

  private updatePathsState = (pathsState: ProjectPathsState) => {
    this.pathsState = pathsState;
  };

  private revealFile = (entryPathRaw: unknown) => {
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

    const [, ...pathParts] = relativeEntryPath.split('\\').reverse();
    pathParts.reverse();

    const entryDirPath = [this.path, ...pathParts].join('\\');

    // Is this file even real?
    if (!this.fs[entryDirPath]?.find((dirEntry) => dirEntry.path === entryPath)) {
      return;
    }

    // Open corresponding paths, if not in project root
    if (entryDirPath !== this.path) {
      let dirPath = this.path;
      const patch: Record<string, boolean> = {};

      while (pathParts.length) {
        dirPath += '\\' + pathParts.shift();

        patch[dirPath] = true;
      }

      this.setPathsState(patch);
    }

    this.select(entryPath);
  };
}
export const ProjectState = new class ProjectState {
  public directoryCreatorUI = new OpenFlag();

  public resourceCreatorDir = '';

  public recentProjects: RecentProject[] = [];

  public pendingFolderDeletions = new Set<string>();

  private projectAlreadyOpening = false;
  private projectObject: ProjectObject | null = null;
  private recentProjectsLoading = true;

  get project(): ProjectObject {
    if (this.projectObject === null) {
      throw new Error('Project is not open');
    }

    return this.projectObject;
  }

  get projectName(): string {
    if (this.projectObject) {
      return this.projectObject.manifest.name;
    }

    return 'No project open';
  }

  get areRecentProjectsLoading(): boolean {
    return this.recentProjectsLoading;
  }

  get isProjectOpening(): boolean {
    return this.projectAlreadyOpening;
  }

  get hasProject(): boolean {
    return this.projectObject !== null;
  }

  constructor() {
    makeAutoObservable(this);

    Api.on(projectApi.recents, this.setRecentProjects);

    Api.on(projectApi.open, this.setOpenProject);
    Api.on(projectApi.close, this.destroyProject);

    Api.on(projectApi.freePendingFolderDeletion, this.freePendingFolderDeletion);

    ShellEvents.on('fxdk:buildProject', this.buildProject);
  }

  private setRecentProjects = (recentProjects) => {
    this.recentProjectsLoading = false;
    this.recentProjects = recentProjects;

    const lastProjectPath = localStorage.getItem('last-project-path');

    // Only open matching last recent project if no project open
    if (!this.hasProject) {
      const [lastProject] = recentProjects;

      if (lastProject && lastProject.path === lastProjectPath) {
        console.log('Opening recent', lastProject);
        this.openProject(lastProject.path);
      }
    }
  };

  readonly openResourceCreator = (dir?: string) => {
    if (dir) {
      this.resourceCreatorDir = dir;
    }

    ShellCommands.invoke(ResourceCommands.OPEN_CREATOR);
  };

  openProject(path: string) {
    if (!this.projectAlreadyOpening) {
      this.destroyProject();

      this.projectAlreadyOpening = true;

      Api.send(projectApi.open, path);
    }
  }

  private setOpenProject = ({ project, pathsState }: ProjectOpenData) => {
    this.destroyProject();

    this.resourceCreatorDir = project.path;
    this.projectAlreadyOpening = false;
    this.projectObject = new ProjectObject(project, pathsState);

    localStorage.setItem('last-project-path', this.project.path);
  };

  public readonly closeProject = () => {
    localStorage.removeItem('last-project-path');

    this.destroyProject();
  };

  private readonly destroyProject = () => {
    if (this.projectObject) {
      this.projectObject.dispose();
      this.projectObject = null;
    }
  };

  readonly buildProject = (overrides?: Partial<APIRQ.ProjectBuild>) => {
    const project = this.projectObject;
    if (!project) {
      return;
    }

    const buildPath = getProjectBuildPathVar(project);
    const useVersioning = getProjectUseVersioningVar(project);
    const deployArtifact = getProjectDeployArtifactVar(project);
    const steamWebApiKey = getProjectSteamWebApiKeyVar(project);
    const tebexSecret = getProjectTebexSecretVar(project);

    if (!buildPath) {
      return ShellCommands.invoke(ProjectBuilderCommands.OPEN);
    }

    Api.send(projectApi.build, {
      buildPath,
      useVersioning,
      deployArtifact,
      steamWebApiKey,
      tebexSecret,
      ...overrides,
    } as APIRQ.ProjectBuild);
  };

  openFile(entry: FilesystemEntry, pinned = false) {
    if (this.hasProject) {
      FXCodeState.openFile(entry.path, pinned);
      this.project.select(entry.path);
    }
  }

  addPendingFolderDeletion(path: string) {
    this.pendingFolderDeletions.add(path);
  }

  private freePendingFolderDeletion = (path: string) => {
    this.pendingFolderDeletions.delete(path);
  };
}();
