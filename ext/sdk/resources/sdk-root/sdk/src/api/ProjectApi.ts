import * as fs from 'fs';
import { ApiClient, AssetCreateRequest, assetManagerTypes, RecentProject} from './api.types';
import { ExplorerApi } from './ExplorerApi';
import * as paths from '../paths';
import { errorsApi, projectApi } from './events';
import { ProjectCreateRequest, ProjectInstance } from './ProjectInstance';
import { createLock, notNull } from './utils';
import { SystemEvent, systemEvents } from './api.events';


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

export class ProjectApi {
  projectInstance: ProjectInstance | null = null;
  projectLock = createLock();

  constructor(
    private readonly client: ApiClient,
    private readonly explorer: ExplorerApi,
  ) {
    this.client.on(projectApi.getRecents, () => this.updateRecentProjects());
    this.client.on(projectApi.removeRecent, (projectPath: string) => this.deleteRecentProject(projectPath));

    this.client.on(projectApi.open, (projectPath: string) => this.openProject(projectPath));
    this.client.on(projectApi.create, async ({ projectPath, name, withServerData }) => {
      try {
        await this.createProject(projectPath, name, withServerData);
      } catch (e) {
        console.error(e);
        this.client.emit(errorsApi.projectCreateError, e.toString());
      }
    });
  }

  async getRecentProjects(): Promise<RecentProject[]> {
    try {
      await fs.promises.stat(paths.recentProjectsFilePath);

      this.client.log('recents file exists');

      const content = await fs.promises.readFile(paths.recentProjectsFilePath);

      return JSON.parse(content.toString('utf8'));
    } catch (e) {
      this.client.log('error reading recents', e);
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

  async openProject(projectPath: string): Promise<ProjectInstance> {
    this.client.log('Opening project', projectPath);

    await this.projectLock.waitForUnlock();

    this.projectLock.lock();

    if (this.projectInstance) {
      await this.projectInstance.close();
    }

    this.projectInstance = await ProjectInstance.openProject(projectPath, this.client, this.explorer);

    this.emitProjectOpen();
    this.setCurrentProjectInstanceAsMostRecent();

    this.projectLock.unlock();

    return this.projectInstance;
  }

  async createProject(projectBasePath: string, name: string, withServerData: boolean = false) {
    this.client.log('Creating project', {
      projectBasePath,
      name,
      withServerData,
    });

    await this.projectLock.waitForUnlock();
    this.projectLock.lock();

    if (this.projectInstance) {
      await this.projectInstance.close();
      this.projectInstance = null;
    }

    const projectCreateRequest: ProjectCreateRequest = {
      path: projectBasePath,
      name,
    };

    const instance = this.projectInstance = await ProjectInstance.createProject(projectCreateRequest, this.client, this.explorer);

    if (withServerData) {
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
