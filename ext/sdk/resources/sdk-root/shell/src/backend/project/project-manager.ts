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
import { ProjectCreateRequest } from "shared/api.requests";
import { ProjectCreateCheckResult, RecentProject } from "shared/project.types";
import { notNull } from "shared/utils";
import { Project } from "./project";
import { ProjectAccess } from "./project-access";
import { assetImporterTypes } from "shared/asset.types";
import { GitAssetImportRequest } from "./asset/importer-contributions/git-importer/git-importer.types";

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
  'webpack',
  'yarn',
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

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  protected project: Project | null = null;

  protected projectOpsSequencer = new Sequencer();

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

  @handlesClientEvent(projectApi.open)
  async openProject(projectPath: string): Promise<Project | null | void> {
    return this.projectOpsSequencer.executeBlocking(async () => {
      this.logService.log('Opening project', projectPath);

      if (this.project) {
        await this.project.unload();
        this.projectAccess.setInstance(null);
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
    if (await this.fsService.statSafe(serverDataPath)) {
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

    await this.projectOpsSequencer.executeBlocking(async () => {
      if (this.project) {
        await this.project.unload();
        this.projectAccess.setInstance(null);
      }

      this.project = await this.createProjectBoundToPath().create(request);

      this.emitProjectOpen();
      this.setCurrentProjectInstanceAsMostRecent();
    });

    if (!checkResult.ignoreCfxServerData) {
      const assetImportRequest: GitAssetImportRequest = {
        importerType: assetImporterTypes.git,
        assetName: 'system-resources',
        assetBasePath: this.project.getPath(),
        assetMetaFlags: {
          readOnly: true,
        },
        data: {
          repoUrl: 'https://github.com/citizenfx/cfx-server-data.git',
        },
        callback: () => {
          this.project?.setResourcesEnabled(cfxServerDataEnabledResources, true);
        },
      };

      this.project.importAsset(assetImportRequest);
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
