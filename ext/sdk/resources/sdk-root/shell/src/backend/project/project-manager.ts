import { inject, injectable } from "inversify";
import { ApiClient } from "backend/api/api-client";
import { ApiContribution, ApiContributionFactory } from "backend/api/api-contribution";
import { handlesClientEvent } from "backend/api/api-decorators";
import { ConfigService } from "backend/config-service";
import { fxdkProjectFilename } from "backend/constants";
import { Sequencer } from "backend/execution-utils/sequencer";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { NotificationService } from "backend/notification/notification-service";
import { projectApi } from "shared/api.events";
import { AssetCreateRequest, ProjectCreateRequest } from "shared/api.requests";
import { assetManagerTypes, ProjectCreateCheckResult, RecentProject } from "shared/api.types";
import { notNull } from "shared/utils";
import { Project } from "./project";

export const cfxServerDataEnabledResources = [
  'basic-gamemode',
  'fivem-map-skater',
  'chat',
  'playernames',
  'mapmanager',
  'spawnmanager',
  'sessionmanager',
  'baseevents',
  'hardcap',
];

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

  protected project: Project | null = null;

  protected projectOpsSequencer = new Sequencer();

  getProject() {
    return this.project;
  }

  async getRecentProjects(): Promise<RecentProject[]> {
    try {
      await this.fsService.stat(this.configService.recentProjectsFilePath);

      this.logService.log('recents file exists');

      const content = await this.fsService.readFile(this.configService.recentProjectsFilePath);

      return JSON.parse(content.toString('utf8'));
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
    return this.apiContributionFactory<Project>(Project);
  }

  @handlesClientEvent(projectApi.open)
  async openProject(projectPath: string): Promise<Project | null | void> {
    return this.projectOpsSequencer.executeBlocking(async () => {
      this.logService.log('Opening project', projectPath);

      if (this.project) {
        await this.project.unload();
      }

      this.project = await this.createProjectBoundToPath().load(projectPath);

      this.emitProjectOpen();
      this.setCurrentProjectInstanceAsMostRecent();

      return this.project;
    });
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

    const serverDataPath = this.fsService.joinPath(projectPath, 'cfx-server-data');
    if (request.withServerData && await this.fsService.statSafe(serverDataPath)) {
      result.ignoreCfxServerData = true;
    }

    return finish();
  }

  @handlesClientEvent(projectApi.create)
  async createProject(request: ProjectCreateRequest) {
    const checkResult = await this.checkCreateRequest(request);

    if (checkResult.openProject) {
      return this.openProject(this.fsService.joinPath(request.projectPath, request.projectName));
    }

    this.logService.log('Creating project', request);

    return this.projectOpsSequencer.executeBlocking(async () => {
      if (this.project) {
        await this.project.unload();
      }

      this.project = await this.createProjectBoundToPath().create(request);

      if (!checkResult.ignoreCfxServerData && request.withServerData) {
        const assetCreateRequest: AssetCreateRequest = {
          assetName: 'cfx-server-data',
          assetPath: this.project.getPath(),
          managerType: assetManagerTypes.git,
          managerData: {
            repoUrl: 'https://github.com/citizenfx/cfx-server-data.git',
          },
          readOnly: true,
          callback: () => {
            this.project?.setResourcesEnabled(cfxServerDataEnabledResources, true);
          },
        };

        this.project.createAsset(assetCreateRequest);
      }

      this.emitProjectOpen();
      this.setCurrentProjectInstanceAsMostRecent();
    });
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

    this.apiClient.emit(projectApi.open, this.project.projectData);
  }

  private setCurrentProjectInstanceAsMostRecent() {
    if (!this.project) {
      return;
    }

    const project = this.project.projectData;

    this.saveMostRecentProject(project.path, project.manifest.name);
  }
}
