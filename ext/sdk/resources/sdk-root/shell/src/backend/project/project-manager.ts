import { inject, injectable } from "inversify";
import { ApiClient } from "backend/api/api-client";
import { ApiContribution, ApiContributionFactory } from "backend/api/api-contribution";
import { handlesClientCallbackEvent, handlesClientEvent } from "backend/api/api-decorators";
import { ConfigService } from "backend/config-service";
import { fxdkProjectFilename } from "backend/constants";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { NotificationService } from "backend/notification/notification-service";
import { projectApi } from "shared/api.events";
import { ProjectCreateRequest } from "shared/api.requests";
import { ProjectCreateCheckResult, RecentProject } from "shared/project.types";
import { notNull } from "shared/utils";
import { Project } from "./project";
import { ProjectAccess } from "./project-access";

@injectable()
export class ProjectManager implements ApiContribution {
  getId() {
    return 'ProjectManager';
  }

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(ApiContributionFactory)
  protected readonly apiContributionFactory: ApiContributionFactory;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  protected project: Project | null = null;
  protected projectLock: boolean = false;

  getProject() {
    return this.project;
  }

  async getRecentProjects(): Promise<RecentProject[]> {
    const recentProjectsFileStat = await this.fsService.statSafe(this.configService.recentProjectsFilePath);
    if (!recentProjectsFileStat) {
      return [];
    }

    try {
      return this.fsService.readFileJson(this.configService.recentProjectsFilePath);
    } catch (e) {
      this.notificationService.warning(`Failed to read recent projects file at ${this.configService.recentProjectsFilePath}, error: ${e.toString()}`);
      return [];
    }
  }

  async setRecentProjects(recentProjects: RecentProject[]): Promise<void> {
    await this.fsService.writeFile(
      this.configService.recentProjectsFilePath,
      JSON.stringify(recentProjects, null, 2),
    );

    this.updateRecentProjects();
  }

  @handlesClientEvent(projectApi.removeRecent)
  async deleteRecentProject(projectPath: string) {
    const recents = await this.getRecentProjects();
    const newRecents = recents.filter((recent) => recent.path !== projectPath);

    await this.setRecentProjects(newRecents);
  }

  async saveMostRecentProject(projectPath: string, name: string): Promise<void> {
    const recentProjects = await this.getRecentProjects();

    // haxx as lazy, ok
    // We need to dedupe recent projects and it's easier with strings, right
    const recentProjectsStrings = recentProjects.map((recentProject) => JSON.stringify(recentProject));
    const projectString = JSON.stringify({ name, path: projectPath });

    const newRecentProjectsStrings = [...new Set([projectString, ...recentProjectsStrings])];
    const newRecentProjects = newRecentProjectsStrings.map((recentProjectString) => JSON.parse(recentProjectString));

    await this.setRecentProjects(newRecentProjects);

    this.logService.log('done writing recent projects', newRecentProjects);
    this.apiClient.emit(projectApi.recents, newRecentProjects);
  }

  protected createProjectBoundToPath(): Project {
    const project = this.apiContributionFactory<Project>(Project);

    this.projectAccess.setInstance(project);

    return project;
  }

  @handlesClientCallbackEvent(projectApi.checkOpenRequest)
  async checkOpenRequest(projectPath: string): Promise<boolean> {
    const projectManifestPath: string = this.fsService.joinPath(projectPath, fxdkProjectFilename);

    return !!(await this.fsService.statSafe(projectManifestPath));
  }

  @handlesClientEvent(projectApi.open)
  async openProject(projectPath: string) {
    if (this.projectLock) {
      throw new Error('Can not open project while another project is being opened or created');
    }

    this.projectLock = true;

    this.logService.log('Opening project', projectPath);

    try {
      if (this.project) {
        await this.project.unload();
        this.projectAccess.setInstance(null);
      }

      this.project = await this.createProjectBoundToPath().load(projectPath);

      this.emitProjectOpen();
      this.setCurrentProjectInstanceAsMostRecent();
    } catch (e) {
      throw e;
    } finally {
      this.projectLock = false;
    }
  }

  @handlesClientEvent(projectApi.checkCreateRequest)
  async checkCreateRequest(request: ProjectCreateRequest): Promise<ProjectCreateCheckResult> {
    const result: ProjectCreateCheckResult = {};
    const finish = () => {
      this.apiClient.emit(projectApi.checkCreateResult, result);

      return result;
    };

    const projectPath = this.fsService.joinPath(request.projectPath, request.projectName);

    // Check if project already exist within given path
    const projectManifestPath = this.fsService.joinPath(projectPath, fxdkProjectFilename);
    if (await this.fsService.statSafe(projectManifestPath)) {
      result.openProject = true;

      return finish();
    }

    return finish();
  }

  @handlesClientEvent(projectApi.create)
  async createProject(request: ProjectCreateRequest) {
    if (this.projectLock) {
      throw new Error('Can not create project while another project is being opened or created');
    }

    this.projectLock = true;

    const checkResult = await this.checkCreateRequest(request);
    if (checkResult.openProject) {
      // Project open will lock it again
      this.projectLock = false;
      await this.openProject(this.fsService.joinPath(request.projectPath, request.projectName));
      return;
    }

    this.logService.log('Creating project', request);

    try {
      if (this.project) {
        await this.project.unload();
        this.projectAccess.setInstance(null);
      }

      this.project = await this.createProjectBoundToPath().create(request);

      this.emitProjectOpen();
      this.setCurrentProjectInstanceAsMostRecent();
    } catch (e) {
      throw e;
    } finally {
      this.projectLock = false;
    }
  }

  @handlesClientEvent(projectApi.getRecents)
  private async updateRecentProjects() {
    const recentProjects = await this.getRecentProjects();

    const newRecentProjects = (await Promise.all(
      recentProjects.map(async (recentProject) => {
        try {
          await this.fsService.stat(recentProject.path);

          return recentProject;
        } catch (e) {
          return null;
        }
      }),
    )).filter(notNull);

    if (recentProjects.length !== newRecentProjects.length) {
      await this.setRecentProjects(newRecentProjects);
      this.apiClient.emit(projectApi.recents, newRecentProjects);
    } else {
      this.apiClient.emit(projectApi.recents, await this.getRecentProjects());
    }
  }

  private emitProjectOpen() {
    if (!this.project) {
      return;
    }

    this.apiClient.emit(projectApi.open, this.project.getProjectData());
  }

  private setCurrentProjectInstanceAsMostRecent() {
    if (!this.project) {
      return;
    }

    this.saveMostRecentProject(this.project.getPath(), this.project.getName());
  }
}
