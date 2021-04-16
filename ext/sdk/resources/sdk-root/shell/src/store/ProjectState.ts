import { makeAutoObservable, reaction } from "mobx";
import { projectApi } from "shared/api.events";
import { ProjectBuildRequest } from "shared/api.requests";
import { FilesystemEntry, FilesystemEntryMap } from "shared/api.types";
import { ProjectData, ProjectFsUpdate, ProjectManifest, ProjectResources, RecentProject } from "shared/project.types";
import { onApiMessage, sendApiMessage } from "utils/api";
import { getProjectBuildPathVar, getProjectDeployArtifactVar, getProjectSteamWebApiKeyVar, getProjectTebexSecretVar, getProjectUseVersioningVar } from "utils/projectStorage";
import { onWindowEvent } from "utils/windowMessages";
import { ShellState } from "./ShellState";
import { TheiaState } from "./TheiaState";


class ProjectObject implements ProjectData {
  public fs: FilesystemEntryMap;

  public path: string;
  public manifest: ProjectManifest;

  public resources: ProjectResources;

  constructor(projectData: ProjectData) {
    this.fs = projectData.fs;
    this.path = projectData.path;
    this.manifest = projectData.manifest;
    this.resources = projectData.resources;

    makeAutoObservable(this);
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

  update(projectData: Partial<ProjectData>) {
    if (projectData.fs) {
      this.fs = projectData.fs;
    }

    if (projectData.manifest) {
      this.manifest = projectData.manifest;
    }

    if (projectData.resources) {
      this.resources = projectData.resources;
    }
  }

  updateFs(update: ProjectFsUpdate) {
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
  }

  updateResources(resources: ProjectResources) {
    this.resources = resources;
  }
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
    onApiMessage(projectApi.update, this.updateProject);
    onApiMessage(projectApi.fsUpdate, this.updateFs);
    onApiMessage(projectApi.resourcesUpdate, this.updateResources);

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
    this.projectAlreadyOpening = false;
    this.projectObject = new ProjectObject(projectData);
    localStorage.setItem('last-project-path', this.project.path);
  };

  private updateProject = (projectData) => {
    this.project?.update(projectData);
  };

  private updateFs = (fs) => {
    this.project?.updateFs(fs);
  };

  private updateResources = (resources) => {
    this.project?.updateResources(resources);
  };

  private closeProject = () => {
    this.projectObject = null;
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
