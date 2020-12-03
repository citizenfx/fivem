import fs from 'fs';
import path from 'path';
import { ApiClient, assetManagerTypes, ProjectCreateCheckResult, RecentProject} from 'shared/api.types';
import { doesPathExist, ExplorerApi } from './ExplorerApi';
import * as paths from '../paths';
import { projectApi } from 'shared/api.events';
import { ProjectInstance } from './ProjectInstance';
import { createLock, notNull } from '../../shared/utils';
import { SystemEvent, systemEvents } from './systemEvents';
import { AssetCreateRequest, ProjectCreateRequest } from 'shared/api.requests';
import { NotificationsApi } from './NotificationsApi';
import { fxdkProjectFilename } from './constants';
import { ApiBase } from './ApiBase';


const cfxServerDataEnabledResources = [
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

export class ProjectApi extends ApiBase {
  projectInstance: ProjectInstance | null = null;
  projectLock = createLock();

  constructor(
    private readonly client: ApiClient,
    private readonly explorer: ExplorerApi,
    private readonly notifications: NotificationsApi,
  ) {
    super();

    this.client.on(projectApi.getRecents, this.bind(this.updateRecentProjects));
    this.client.on(projectApi.removeRecent, this.bind(this.deleteRecentProject));

    this.client.on(projectApi.open, this.bind(this.openProject));
    this.client.on(projectApi.create, this.bind(this.createProject));
    this.client.on(projectApi.checkCreateRequest, this.bind(this.checkCreateRequest));
  }

  async getRecentProjects(): Promise<RecentProject[]> {
    try {
      await fs.promises.stat(paths.recentProjectsFilePath);

      this.client.log('recents file exists');

      const content = await fs.promises.readFile(paths.recentProjectsFilePath);

      return JSON.parse(content.toString('utf8'));
    } catch (e) {
      this.notifications.warning(`Failed to read recent projects file at ${paths.recentProjectsFilePath}, error: ${e.toString()}`);
      return [];
    }
  }

  async setRecentProjects(recentProjects: RecentProject[]): Promise<void> {
    await fs.promises.writeFile(
      paths.recentProjectsFilePath,
      JSON.stringify(recentProjects, null, 2),
    );

    this.updateRecentProjects();
  }

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

    this.client.log('done writing recent projects', newRecentProjects);
    this.client.emit(projectApi.recents, newRecentProjects);
  }

  async openProject(projectPath: string): Promise<ProjectInstance | null | void> {
    return this.projectLock.withExclusiveLock(async () => {
      this.client.log('Opening project', projectPath);

      if (this.projectInstance) {
        await this.projectInstance.close();
      }

      this.projectInstance = await ProjectInstance.openProject(projectPath, this.client, this.explorer);

      this.emitProjectOpen();
      this.setCurrentProjectInstanceAsMostRecent();

      return this.projectInstance;
    });
  }

  async checkCreateRequest(request: ProjectCreateRequest): Promise<ProjectCreateCheckResult> {
    const result: ProjectCreateCheckResult = {};
    const finish = () => {
      this.client.emit(projectApi.checkCreateResult, result);

      return result;
    };

    const projectPath = path.join(request.projectPath, request.projectName);

    // Check if project already exist within given path
    const projectManifestPath = path.join(projectPath, fxdkProjectFilename);
    if (await doesPathExist(projectManifestPath)) {
      result.openProject = true;

      return finish();
    }

    const serverDataPath = path.join(projectPath, 'cfx-server-data');
    if (request.withServerData && await doesPathExist(serverDataPath)) {
      result.ignoreCfxServerData = true;
    }

    return finish();
  }

  async createProject(request: ProjectCreateRequest) {
    const checkResult = await this.checkCreateRequest(request);

    if (checkResult.openProject) {
      return this.openProject(path.join(request.projectPath, request.projectName));
    }

    this.client.log('Creating project', request);

    await this.projectLock.waitForUnlock();
    this.projectLock.lock();

    if (this.projectInstance) {
      await this.projectInstance.close();
      this.projectInstance = null;
    }

    const instance = this.projectInstance = await ProjectInstance.createProject(request, this.client, this.explorer);

    if (!checkResult.ignoreCfxServerData && request.withServerData) {
      const assetCreateRequest: AssetCreateRequest = {
        assetName: 'cfx-server-data',
        assetPath: instance.project.path,
        managerType: assetManagerTypes.git,
        managerData: {
          repoUrl: 'https://github.com/citizenfx/cfx-server-data.git',
        },
        readOnly: true,
        callback: () => {
          instance.setResourcesEnabled(cfxServerDataEnabledResources, true);
        },
      };

      systemEvents.emit(SystemEvent.assetCreateRequest, assetCreateRequest);
    }

    this.emitProjectOpen();
    this.setCurrentProjectInstanceAsMostRecent();

    this.projectLock.unlock();
  }

  private async updateRecentProjects() {
    const recentProjects = await this.getRecentProjects();

    const newRecentProjects = (await Promise.all(
      recentProjects.map(async (recentProject) => {
        try {
          await fs.promises.stat(recentProject.path);

          return recentProject;
        } catch (e) {
          return null;
        }
      }),
    )).filter(notNull);

    if (recentProjects.length !== newRecentProjects.length) {
      await this.setRecentProjects(newRecentProjects);
      this.client.emit(projectApi.recents, newRecentProjects);
    } else {
      this.client.emit(projectApi.recents, await this.getRecentProjects());
    }
  }

  private emitProjectOpen() {
    if (!this.projectInstance) {
      return;
    }

    this.client.emit(projectApi.open, this.projectInstance.project);
  }

  private setCurrentProjectInstanceAsMostRecent() {
    if (!this.projectInstance) {
      return;
    }

    const project = this.projectInstance.project;

    this.saveMostRecentProject(project.path, project.manifest.name);
  }
}
