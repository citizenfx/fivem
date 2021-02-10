import { ApiClient } from 'backend/api/api-client';
import { ApiContribution } from 'backend/api/api-contribution';
import { handlesClientEvent } from 'backend/api/api-decorators';
import { CopyOptions, FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { Task, TaskReporterService } from 'backend/task/task-reporter-service';
import { Stats } from 'fs';
import { injectable, inject } from 'inversify';
import { projectApi } from 'shared/api.events';
import { ProjectBuildRequest } from 'shared/api.requests';
import { ProjectBuildError } from 'shared/api.types';
import { projectBuildingTaskName, ProjectBuildTaskStage } from 'shared/task.names';
import { formatDateForFilename } from 'utils/date';
import { AssetBuildCommandError } from './asset/asset-error';
import { Project, ProjectState } from './project';
import { ProjectAccess } from './project-access';

class BuildError {
  constructor(
    public readonly message: string,
  ) {

  }
}

interface ProjectBuildInfo {
  task: Task<ProjectBuildTaskStage>,
  buildPath: string,
  useVersioning: boolean,
  deployPath: string,
  deployPathStat: Stats | null,
}

@injectable()
export class ProjectBuilder implements ApiContribution {
  getId() {
    return 'ProjectBuilder';
  }

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @handlesClientEvent(projectApi.build)
  public async build(request: ProjectBuildRequest) {
    if (!this.projectAccess.hasInstance()) {
      this.notificationService.error('Project build failed: no current project');

      return;
    }

    const project = this.projectAccess.getInstance();
    if (project.isNotInDevState()) {
      this.notificationService.error('Project build failed: project is already building');

      return;
    }

    const task = this.taskReporterService.createNamed<ProjectBuildTaskStage>(projectBuildingTaskName, 'Project building');

    try {
      project.enterState(ProjectState.Building);
      await this.doBuildProject(request, project, task);
    } catch (e) {
      let projectBuildError: ProjectBuildError = {
        type: 'generic',
        data: e.toString(),
      };

      if (e instanceof AssetBuildCommandError) {
        projectBuildError = {
          type: 'assetBuildError',
          data: {
            assetName: e.assetName,
            outputChannelId: e.outputChannelId,
          },
        };
      }

      this.apiClient.emit(projectApi.buildError, projectBuildError);
    } finally {
      project.enterState(ProjectState.Development);
      task.done();
    }
  }

  protected async doBuildProject(request: ProjectBuildRequest, project: Project, task: Task<ProjectBuildTaskStage>) {
    const { useVersioning, buildPath } = request;

    const buildInfo: ProjectBuildInfo = {
      task,
      buildPath,
      useVersioning,
      deployPath: this.fsService.joinPath(buildPath, 'resources'),
      deployPathStat: null,
    };

    task.setStage(ProjectBuildTaskStage.VerifyingBuildSite)
    await this.checkBuildSite(buildInfo);

    task.setStage(ProjectBuildTaskStage.StoppingWatchCommands);
    task.setText('Stopping all watch commands');
    await project.suspendWatchCommands();

    task.setStage(ProjectBuildTaskStage.RunningBuildCommands);
    task.setText('Running build commands');
    await project.runBuildCommands();

    task.setStage(ProjectBuildTaskStage.PreparingBuildSite);
    task.setText('Preparing build site');
    await this.prepareBuildSite(buildInfo);

    task.setStage(ProjectBuildTaskStage.DeployingToBuildSite);
    task.setText('Deploying to build site');
    await this.deployToBuildSite(project, buildInfo);

    task.setStage(ProjectBuildTaskStage.Done);
    task.setText('Starting watch commands');
    project.enterState(ProjectState.Development);
    project.resumeWatchCommands();

    this.notificationService.info('Project build completed', 5000);
  }

  protected getDeployPath(buildPath: string) {
    return this.fsService.joinPath(buildPath, 'resources');
  }

  protected async checkBuildSite(buildInfo: ProjectBuildInfo) {
    const buildPathStat = await this.fsService.statSafe(buildInfo.buildPath);
    if (!buildPathStat) {
      throw new BuildError('Build path does not exist');
    }

    buildInfo.deployPathStat = await this.fsService.statSafe(buildInfo.deployPath);
    if (buildInfo.deployPathStat && !buildInfo.deployPathStat.isDirectory()) {
      throw new BuildError('Build path contains "resources" entry that is not directory');
    }
  }

  protected async prepareBuildSite(buildInfo: ProjectBuildInfo) {
    const deployPathExists = buildInfo.deployPathStat !== null;
    if (deployPathExists) {
      // If versioned build - back up old deploy
      if (buildInfo.useVersioning) {
        const newName = this.fsService.joinPath(buildInfo.buildPath, `resources_${formatDateForFilename(new Date())}`);

        await this.fsService.rename(buildInfo.deployPath, newName);
      } else {
        await this.fsService.rimraf(buildInfo.deployPath);
      }
    }

    await this.fsService.mkdirp(buildInfo.deployPath);
    buildInfo.deployPathStat = await this.fsService.statSafe(buildInfo.buildPath);
  }

  protected async deployToBuildSite(project: Project, buildInfo: ProjectBuildInfo) {
    const { deployPath } = buildInfo;
    const deployableResources = project.getEnabledResourcesAssets();

    const resourcesCount = deployableResources.length;

    await Promise.all(
      deployableResources.map(async (resourceAsset) => {
        const resourceName = resourceAsset.getName();
        const resourcePath = resourceAsset.getPath();
        const deployablePaths = await resourceAsset.getDeployablePaths();

        const copyTasks: [string, string][] = [];
        const foldersToCreate: Set<string> = new Set();

        deployablePaths.forEach((deployablePath: string) => {
          const relativePath = this.fsService.dirname(this.fsService.relativePath(resourcePath, deployablePath));
          const deploySitePath = this.fsService.joinPath(deployPath, resourceName, relativePath);

          // Remember, that it goes like so: /a/b/c.txt -> /a/d, so it will get copied to /a/d/c.txt
          copyTasks.push([deployablePath, deploySitePath]);
          foldersToCreate.add(deploySitePath);

          this.logService.log('Deploying', deployablePath, 'to', deploySitePath);
        });

        // Create folders in a sequence rather than in parallel as mkdirp is misbehaving if run in parallel and overlapses in paths present
        for (const folderToCreate of foldersToCreate) {
          await this.fsService.mkdirp(folderToCreate);
        }

        const progressQuant = (1 / resourcesCount) / copyTasks.length;
        const copyOptions: CopyOptions = {
          onProgress: (progress) => buildInfo.task.setProgress(progress * progressQuant),
        };

        // Now copy files
        await Promise.all(
          copyTasks.map(([sourcePath, target]) => this.fsService.copy(sourcePath, target, copyOptions)),
        );
      }),
    );
  }
}
