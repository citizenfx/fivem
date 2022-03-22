import { inject, injectable, postConstruct } from "inversify";
import { ApiClient } from "backend/api/api-client";
import { ApiContribution, ApiContributionFactory } from "backend/api/api.extensions";
import { handlesClientCallbackEvent, handlesClientEvent } from "backend/api/api-decorators";
import { ConfigService } from "backend/config-service";
import { fxdkProjectFilename } from "backend/constants";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { NotificationService } from "backend/notification/notification-service";
import { APIRQ } from "shared/api.requests";
import { ProjectCreateCheckResult, RecentProject } from "shared/project.types";
import { notNull } from "shared/utils";
import { ProjectInstanceService } from "./project";
import { ProjectAccess } from "./project-access";
import { ProjectApi } from "../common/project.api";

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

  protected project: ProjectInstanceService | null = null;
  protected projectLock: boolean = false;

  @postConstruct()
  initialize() {
    this.apiClient.onClientConnected.addListener(() => {
      this.updateRecentProjects();
    });
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

  @handlesClientEvent(ProjectApi.LoaderEndpoints.removeRecent)
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

    this.apiClient.emit(ProjectApi.LoaderEndpoints.recents, newRecentProjects);
  }

  protected createProjectInstance(): ProjectInstanceService {
    const project = this.apiContributionFactory<ProjectInstanceService>(ProjectInstanceService);

    this.projectAccess.setInstance(project);

    return project;
  }

  @handlesClientCallbackEvent(ProjectApi.LoaderEndpoints.checkOpenRequest)
  async checkOpenRequest(projectPath: string): Promise<boolean> {
    const projectManifestPath: string = this.fsService.joinPath(projectPath, fxdkProjectFilename);

    return !!(await this.fsService.statSafe(projectManifestPath));
  }

  @handlesClientEvent(ProjectApi.LoaderEndpoints.open)
  async openProject(projectPath: string) {
    if (this.projectLock) {
      throw new Error('Can not open project while another project is being opened or created');
    }

    this.projectLock = true;

    try {
      await this.unloadCurrentProject();

      this.project = await this.createProjectInstance().open(projectPath);

      this.emitProjectOpen();
      this.setCurrentProjectInstanceAsMostRecent();
    } catch (e) {
      throw e;
    } finally {
      this.projectLock = false;
    }
  }

  @handlesClientEvent(ProjectApi.LoaderEndpoints.checkCreateRequest)
  async checkCreateRequest(request: APIRQ.ProjectCreate): Promise<ProjectCreateCheckResult> {
    const result: ProjectCreateCheckResult = {};
    const finish = () => {
      this.apiClient.emit(ProjectApi.LoaderEndpoints.checkCreateResult, result);

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

  @handlesClientEvent(ProjectApi.LoaderEndpoints.create)
  async createProject(request: APIRQ.ProjectCreate) {
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

    try {
      await this.unloadCurrentProject();

      this.project = await this.createProjectInstance().create(request);

      this.emitProjectOpen();
      this.setCurrentProjectInstanceAsMostRecent();
    } catch (e) {
      throw e;
    } finally {
      this.projectLock = false;
    }
  }

  private async unloadCurrentProject() {
    if (this.project) {
      await this.projectAccess.getNoUsagePromise();
      await this.project.unload();
      this.projectAccess.setInstance(null);
    }
  }

  @handlesClientEvent(ProjectApi.LoaderEndpoints.getRecents)
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
      this.apiClient.emit(ProjectApi.LoaderEndpoints.recents, newRecentProjects);
    } else {
      this.apiClient.emit(ProjectApi.LoaderEndpoints.recents, await this.getRecentProjects());
    }
  }

  private emitProjectOpen() {
    if (!this.project) {
      return;
    }

    this.apiClient.emit(ProjectApi.LoaderEndpoints.open, this.project.getProjectOpenData());
  }

  private setCurrentProjectInstanceAsMostRecent() {
    if (!this.project) {
      return;
    }

    this.saveMostRecentProject(this.project.getPath(), this.project.getName());
  }
}
