import { Api } from "fxdk/browser/Api";
import { ProjectApi } from "fxdk/project/common/project.api";
import { makeAutoObservable } from "mobx";
import { ProjectOpenData, RecentProject } from "shared/project.types";
import { disposeProjectInstance, setProjectInstance } from "./project";
import { ProjectInstance } from "./projectInstance";
import { ProjectStateEvents } from "./projectStateEvents";

export const ProjectLoader = new class ProjectLoader {
  private _projectOpen = false;
  private _projectOpening = false;

  private _recentsLoading = true;
  private _recents: RecentProject[] = [];

  get recentProjectLoaded(): boolean {
    return !this._recentsLoading;
  }

  get recentProjects(): ReadonlyArray<RecentProject> {
    return this._recents;
  }

  get hasProject(): boolean {
    return this._projectOpen;
  }

  get isProjectOpening(): boolean {
    return this._projectOpening;
  }

  constructor() {
    makeAutoObservable(this);

    Api.on(ProjectApi.LoaderEndpoints.recents, this.handleRecents);

    Api.on(ProjectApi.LoaderEndpoints.open, this.handleProjectOpen);
    Api.on(ProjectApi.LoaderEndpoints.close, this.handleProjectClose);
  }

  open(projectPath: string) {
    if (this._projectOpening) {
      return;
    }

    this.handleProjectClose();

    this._projectOpening = true;

    Api.send(ProjectApi.LoaderEndpoints.open, projectPath);
  }

  readonly close = () => {
    this.handleProjectClose();

    Api.send(ProjectApi.LoaderEndpoints.close);
  };

  private handleRecents = (recentProjects: RecentProject[]) => {
    this._recentsLoading = false;
    this._recents = recentProjects;

    const lastProjectPath = localStorage.getItem('last-project-path');

    // Only open matching last recent project if no project open
    if (!this.hasProject) {
      const [lastProject] = recentProjects;

      if (lastProject && lastProject.path === lastProjectPath) {
        console.log('Opening recent', lastProject);
        this.open(lastProject.path);
      }
    }
  };

  private handleProjectClose = () => {
    if (this._projectOpen) {
      ProjectStateEvents.BeforeClose.emit();

      this._projectOpen = false;

      disposeProjectInstance();

      localStorage.removeItem('last-project-path');
    }
  };

  private handleProjectOpen = (openData: ProjectOpenData) => {
    setProjectInstance(new ProjectInstance(openData));

    this._projectOpening = false;
    this._projectOpen = true;

    localStorage.setItem('last-project-path', openData.path);
  };
}();
