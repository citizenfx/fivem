import { DisposableContainer, DisposableObject } from "backend/disposable-container";
import { makeAutoObservable, reaction } from "mobx";
import { assetApi, projectApi } from "shared/api.events";
import { ProjectBuildRequest } from "shared/api.requests";
import { FilesystemEntry, FilesystemEntryMap } from "shared/api.types";
import { AssetType } from "shared/asset.types";
import { ProjectAssetBaseConfig, ProjectData, ProjectFsUpdate, ProjectManifest, RecentProject } from "shared/project.types";
import { onApiMessage, sendApiMessage } from "utils/api";
import { getProjectBuildPathVar, getProjectDeployArtifactVar, getProjectSteamWebApiKeyVar, getProjectTebexSecretVar, getProjectUseVersioningVar } from "utils/projectStorage";
import { onWindowEvent } from "utils/windowMessages";
import { ShellState } from "./ShellState";
import { TheiaState } from "../personalities/TheiaPersonality/TheiaState";


class ProjectObject implements ProjectData, DisposableObject {
  public fs: FilesystemEntryMap;

  public path: string;
  public assets: {
    [path: string]: ProjectAssetBaseConfig,
  };
  public manifest: ProjectManifest;

  public assetTypes: Record<string, AssetType | void> = {};

  private disposableContainer: DisposableContainer;

  constructor(projectData: ProjectData) {
    this.fs = projectData.fs;
    this.path = projectData.path;
    this.assets = projectData.assets;
    this.manifest = projectData.manifest;
    this.assetTypes = projectData.assetTypes;

    makeAutoObservable(this);

    this.disposableContainer = new DisposableContainer();

    this.disposableContainer.add(
      onApiMessage(projectApi.update, this.update),
      onApiMessage(projectApi.fsUpdate, this.updateFs),
      onApiMessage(assetApi.setConfig, this.setAssetConfig),
      onApiMessage(assetApi.setType, this.setAssetType),
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
}
export const ProjectState = new class ProjectState {
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

  public recentProjects: RecentProject[] = [];
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

  public creatorOpen = false;
  openCreator = () => this.creatorOpen = true;
  closeCreator = () => this.creatorOpen = false;

  public openerOpen = false;
  openOpener = () => this.openerOpen = true;
  closeOpener = () => this.openerOpen = false;

  public settingsOpen = false;
  openSettings = () => this.settingsOpen = true;
  closeSettings = () => this.settingsOpen = false;

  public builderOpen = false;
  openBuilder = () => this.builderOpen = true;
  closeBuilder = () => this.builderOpen = false;

  public resourceCreatorDir = '';
  setResourceCreatorDir(dir: string) {
    this.resourceCreatorDir = dir;
  }

  public resourceCreatorOpen = false;
  openResourceCreator = (dir?: string) => {
    if (dir) {
      this.resourceCreatorDir = dir;
    }

    this.resourceCreatorOpen = true;
  };
  closeResourceCreator = () => this.resourceCreatorOpen = false;

  public importerOpen = false;
  openImporter = () => this.importerOpen = true;
  closeImporter = () => this.importerOpen = false;

  public directoryCreatorOpen = false;
  openDirectoryCreator = () => this.directoryCreatorOpen = true;
  closeDirectoryCreator = () => this.directoryCreatorOpen = false;

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

  private projectAlreadyOpening = false;
  openProject(path: string) {
    if (!this.projectAlreadyOpening) {
      this.openerOpen = false;
      this.projectAlreadyOpening = true;

      sendApiMessage(projectApi.open, path);
    }
  }

  private setOpenProject = (projectData: ProjectData) => {
    if (this.projectObject) {
      this.projectObject.dispose();
      this.projectObject = null;
    }

    this.projectAlreadyOpening = false;
    this.projectObject = new ProjectObject(projectData);

    localStorage.setItem('last-project-path', this.project.path);
  };

  private closeProject = () => {
    if (this.projectObject) {
      this.projectObject.dispose();
      this.projectObject = null;
    }
  };

  buildProject(overrides?: Partial<ProjectBuildRequest>) {
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
      return this.openBuilder();
    }

    sendApiMessage(projectApi.build, {
      buildPath,
      useVersioning,
      deployArtifact,
      steamWebApiKey,
      tebexSecret,
      ...overrides,
    } as ProjectBuildRequest);
  }

  openFile(entry: FilesystemEntry) {
    if (this.hasProject) {
      TheiaState.openFile(entry.path);
    }
  }

  public pendingFolderDeletions = new Set<string>();

  addPendingFolderDeletion(path: string) {
    this.pendingFolderDeletions.add(path);
  }

  private freePendingFolderDeletion = (path: string) => {
    this.pendingFolderDeletions.delete(path);
  };
};
