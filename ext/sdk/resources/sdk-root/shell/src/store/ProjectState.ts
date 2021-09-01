import { DisposableContainer, DisposableObject } from "backend/disposable-container";
import { makeAutoObservable, reaction } from "mobx";
import { assetApi, projectApi } from "shared/api.events";
import { APIRQ } from "shared/api.requests";
import { FilesystemEntry, FilesystemEntryMap } from "shared/api.types";
import { AssetType } from "shared/asset.types";
import { ProjectAssetBaseConfig, ProjectData, ProjectFsUpdate, ProjectManifest, ProjectOpenData, ProjectPathsState, RecentProject } from "shared/project.types";
import { onApiMessage, sendApiMessage, sendApiMessageCallback } from "utils/api";
import { getProjectBuildPathVar, getProjectDeployArtifactVar, getProjectSteamWebApiKeyVar, getProjectTebexSecretVar, getProjectUseVersioningVar } from "utils/projectStorage";
import { onWindowEvent } from "utils/windowMessages";
import { ShellState } from "./ShellState";
import { TheiaState } from "../personalities/TheiaPersonality/TheiaState";
import { SystemResource } from "backend/system-resources/system-resources-constants";
import { OpenFlag } from "./generic/OpenFlag";
import { ConfirmationsState } from "./ConfirmationsState";
import { deleteIcon } from "constants/icons";
import React from "react";


class ProjectObject implements ProjectData, DisposableObject {
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
      onApiMessage(projectApi.update, this.update),
      onApiMessage(projectApi.fsUpdate, this.updateFs),
      onApiMessage(projectApi.pathsStateUpdate, this.updatePathsState),
      onApiMessage(assetApi.setConfig, this.setAssetConfig),
      onApiMessage(assetApi.setType, this.setAssetType),
      onApiMessage(assetApi.setDefinition, this.setAssetDefinition),
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

    sendApiMessage(projectApi.setSystemResources, this.manifest.systemResources);
  }

  readonly setPathState = (path: string, state: boolean) => {
    this.pathsState[path] = state;

    sendApiMessage(projectApi.setPathsStatePatch, {
      [path]: state,
    });
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
    sendApiMessageCallback(projectApi.deleteEntry, { entryPath }, (error, response) => {
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

            sendApiMessageCallback(projectApi.deleteEntry, request, (error) => {
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

  private setAssetType = (assetTypes: Record<string, AssetType | void>) => {
    for (const [assetPath, assetType] of Object.entries(assetTypes)) {
      this.assetTypes[assetPath] = assetType;
    }
  };

  private setAssetConfig = ([assetPath, config]) => {
    this.assets[assetPath] = config;
  };

  private setAssetDefinition = (assetDefs: Record<string, any>) => {
    for (const [assetPath, def] of Object.entries(assetDefs)) {
      this.assetDefs[assetPath] = def;
    }
  };

  private updatePathsState = (pathsState: ProjectPathsState) => {
    this.pathsState = pathsState;
  };
}
export const ProjectState = new class ProjectState {
  public creatorUI = new OpenFlag();
  public openerUI = new OpenFlag();
  public settingsUI = new OpenFlag();
  public builderUI = new OpenFlag();
  public importerUI = new OpenFlag();
  public directoryCreatorUI = new OpenFlag();
  public mapCreatorUI = new OpenFlag();

  public resourceCreatorDir = '';
  public resourceCreatorUI = new OpenFlag();

  public recentProjects: RecentProject[] = [];

  public pendingFolderDeletions = new Set<string>();

  private projectAlreadyOpening = false;
  private projectObject: ProjectObject | null = null;

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

  get hasProject(): boolean {
    return this.projectObject !== null;
  }

  constructor() {
    makeAutoObservable(this);

    reaction(
      () => ShellState.isReady,
      (isReady) => {
        if (isReady) {
          sendApiMessage(projectApi.getRecents);
        }
      },
    );

    onApiMessage(projectApi.recents, this.setRecentProjects);

    onApiMessage(projectApi.open, this.setOpenProject);
    onApiMessage(projectApi.close, this.closeProject);

    onApiMessage(projectApi.freePendingFolderDeletion, this.freePendingFolderDeletion);

    onWindowEvent('fxdk:buildProject', () => {
      this.buildProject();
    });
  }

  private setRecentProjects = (recentProjects) => {
    this.recentProjects = recentProjects;

    const lastProjectPath = localStorage.getItem('last-project-path');

    // Only open matching last recent project if no project open
    if (!this.hasProject) {
      const [lastProject] = recentProjects;

      if (lastProject && lastProject.path === lastProjectPath) {
        this.openProject(lastProject.path);
      }
    }
  };

  readonly openResourceCreator = (dir?: string) => {
    if (dir) {
      this.resourceCreatorDir = dir;
    }

    this.resourceCreatorUI.open();
  };

  openProject(path: string) {
    if (!this.projectAlreadyOpening) {
      this.closeProject();

      this.openerUI.close();
      this.projectAlreadyOpening = true;

      sendApiMessage(projectApi.open, path);
    }
  }

  private setOpenProject = ({ project, pathsState }: ProjectOpenData) => {
    this.closeProject();

    this.resourceCreatorDir = project.path;
    this.projectAlreadyOpening = false;
    this.projectObject = new ProjectObject(project, pathsState);

    localStorage.setItem('last-project-path', this.project.path);
  };

  private closeProject = () => {
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
      return this.builderUI.open();
    }

    sendApiMessage(projectApi.build, {
      buildPath,
      useVersioning,
      deployArtifact,
      steamWebApiKey,
      tebexSecret,
      ...overrides,
    } as APIRQ.ProjectBuild);
  };

  openFile(entry: FilesystemEntry) {
    if (this.hasProject) {
      TheiaState.openFile(entry.path);
    }
  }

  addPendingFolderDeletion(path: string) {
    this.pendingFolderDeletions.add(path);
  }

  private freePendingFolderDeletion = (path: string) => {
    this.pendingFolderDeletions.delete(path);
  };
}();
