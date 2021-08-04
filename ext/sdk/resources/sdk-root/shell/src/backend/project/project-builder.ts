import { AssetDeployablePathsDescriptor } from 'assets/core/asset-interface';
import { ApiClient } from 'backend/api/api-client';
import { ApiContribution } from 'backend/api/api-contribution';
import { handlesClientEvent } from 'backend/api/api-decorators';
import { CopyOptions, FsService } from 'backend/fs/fs-service';
import { versionFilename } from 'backend/game-server/game-server-installer-utils';
import { GameServerManagerService } from 'backend/game-server/game-server-manager-service';
import { ServerResourceDescriptor } from 'backend/game-server/game-server-runtime';
import { GameServerService } from 'backend/game-server/game-server-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { SystemResourcesService } from 'backend/system-resources/system-resources-service';
import { Task, TaskReporterService } from 'backend/task/task-reporter-service';
import { Stats } from 'fs';
import { injectable, inject } from 'inversify';
import { projectApi } from 'shared/api.events';
import { ProjectBuildRequest } from 'shared/api.requests';
import { ServerUpdateChannel, ServerUpdateStates } from 'shared/api.types';
import { ProjectBuildError } from 'shared/project.types';
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

  steamWebApiKey: string,
  tebexSecret: string,

  serverUpdateChannel: ServerUpdateChannel,

  lastVersion: string,

  useVersioning: boolean,
  deployArtifact: boolean,

  artifactDeployPath: string,
  artifactDeployPathStats: Stats | null,

  resourcesDeployPath: string,
  resourcesDeployPathStats: Stats | null,
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

  @inject(GameServerService)
  protected readonly gameServerService: GameServerService;

  @inject(GameServerManagerService)
  protected readonly gameServerManagerService: GameServerManagerService;

  @inject(SystemResourcesService)
  protected systemResourcesService: SystemResourcesService;

  @handlesClientEvent(projectApi.build)
  public async build(request: ProjectBuildRequest) {
    if (!this.projectAccess.hasInstance()) {
      this.notificationService.error('Project build failed: no current project');

      return;
    }

    const project = this.projectAccess.getInstance();
    if (!project.isInDevState()) {
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
    const { useVersioning, buildPath, deployArtifact, steamWebApiKey, tebexSecret } = request;

    if (project.getManifest().systemResources.length > 0 && !(await this.systemResourcesService.getAvailablePromise())) {
      throw new Error(`System resources unavailable, can't build project`);
    }

    this.logService.log('Deploying project', request);

    const buildInfo: ProjectBuildInfo = {
      task,
      buildPath,
      tebexSecret,
      steamWebApiKey: steamWebApiKey || 'none',
      lastVersion: formatDateForFilename(new Date()),
      serverUpdateChannel: project.getManifest().serverUpdateChannel,
      useVersioning,
      deployArtifact,
      artifactDeployPath: this.fsService.joinPath(buildPath, 'artifact'),
      artifactDeployPathStats: null,
      resourcesDeployPath: this.fsService.joinPath(buildPath, 'resources'),
      resourcesDeployPathStats: null,
    };

    task.setStage(ProjectBuildTaskStage.VerifyingBuildSite)
    await this.checkBuildSite(buildInfo);

    task.setStage(ProjectBuildTaskStage.RunningBuildCommands);
    task.setText('Running build commands');
    await project.runBuildCommands();

    task.setStage(ProjectBuildTaskStage.PreparingBuildSite);
    task.setText('Preparing build site');
    await this.prepareBuildSite(buildInfo);

    task.setStage(ProjectBuildTaskStage.DeployingToBuildSite);
    task.setText('Deploying resources to build site');
    await this.deployResources(project, buildInfo);
    await this.deployArtifact(buildInfo);
    await this.deployMiscFiles(project, buildInfo);

    task.setStage(ProjectBuildTaskStage.Done);
    task.setText('Done');
    project.enterState(ProjectState.Development);

    this.notificationService.info('Project build completed', 5000);
  }

  protected getDeployPath(buildPath: string) {
    return this.fsService.joinPath(buildPath, 'resources');
  }

  protected async checkBuildSite(buildInfo: ProjectBuildInfo) {
    const buildPathStat = await this.fsService.statSafe(buildInfo.buildPath);
    if (!buildPathStat) {
      await this.fsService.mkdirp(buildInfo.buildPath);
    }

    buildInfo.resourcesDeployPathStats = await this.fsService.statSafe(buildInfo.resourcesDeployPath);
    if (buildInfo.resourcesDeployPathStats && !buildInfo.resourcesDeployPathStats.isDirectory()) {
      throw new BuildError('Build path contains "resources" entry that is not directory');
    }

    buildInfo.artifactDeployPathStats = await this.fsService.statSafe(buildInfo.artifactDeployPath);
    if (buildInfo.artifactDeployPathStats && !buildInfo.artifactDeployPathStats.isDirectory()) {
      throw new BuildError('Build path contains "artifact" entry that is not directory');
    }
  }

  protected async prepareBuildSite(buildInfo: ProjectBuildInfo) {
    buildInfo.resourcesDeployPathStats = await this.prepareDeployPath(
      buildInfo.useVersioning,
      buildInfo.resourcesDeployPath,
      buildInfo.resourcesDeployPathStats,
      `resources_${buildInfo.lastVersion}`,
    );

    if (buildInfo.deployArtifact && await this.shouldDeployNewServerArtifact(buildInfo)) {
      buildInfo.artifactDeployPathStats = await this.prepareDeployPath(
        buildInfo.useVersioning,
        buildInfo.artifactDeployPath,
        buildInfo.artifactDeployPathStats,
        `artifact_${buildInfo.lastVersion}`,
      );
    } else {
      buildInfo.deployArtifact = false;
    }
  }

  protected async prepareDeployPath(useVersioning: boolean, deployPath: string, deployPathStats: Stats | null, newName: string): Promise<Stats> {
    const deployPathExists = deployPathStats !== null;
    if (deployPathExists) {
      // If versioned build - back up old deploy
      if (useVersioning) {
        const newPath = this.fsService.joinPath(this.fsService.dirname(deployPath), newName);

        await this.fsService.rename(deployPath, newPath);
      } else {
        await this.fsService.rimraf(deployPath);
      }
    }

    await this.fsService.mkdirp(deployPath);
    return this.fsService.statSafe(deployPath);
  }

  protected async deployResources(project: Project, buildInfo: ProjectBuildInfo) {
    const { resourcesDeployPath } = buildInfo;
    const deployableResources: [ServerResourceDescriptor, AssetDeployablePathsDescriptor][] = project.getEnabledAssets()
      .map((asset) => [asset.getResourceDescriptor?.(), asset.getDeployablePathsDescriptor?.()])
      .filter(([assetResourceDescriptor]) => !!assetResourceDescriptor) as any; // thanks ts, again

    const resourceConfig: string[] = [];

    if (project.getManifest().systemResources.length > 0) {
      const systemResourcesDeployPath = this.fsService.joinPath(resourcesDeployPath, '[system]');
      const systemResourcesDescriptors = this.systemResourcesService.getResourceDescriptors(project.getManifest().systemResources);

      await this.fsService.mkdirp(systemResourcesDeployPath);

      await Promise.all(
        systemResourcesDescriptors.map(({ name, path }) => this.fsService.copyDirContent(
          path,
          this.fsService.joinPath(systemResourcesDeployPath, name),
        )),
      );

      resourceConfig.push('ensure [system]');
    }

    const resourcesCount = deployableResources.length;
    if (resourcesCount) {
      await Promise.all(
        deployableResources.map(async ([resourceDescriptor, deployablePathsDescriptorPromise]) => {
          const deployablePathsDescriptor = await deployablePathsDescriptorPromise;

          if (!deployablePathsDescriptor) {
            this.logService.log(`Skipped deploying of resource as it has no deployable paths descriptor`, resourceDescriptor);
            return;
          }

          const copyTasks: [string, string][] = [];
          const foldersToCreate: Set<string> = new Set();

          deployablePathsDescriptor.paths.forEach((deployablePath: string) => {
            const assetSitePath = this.fsService.joinPath(deployablePathsDescriptor.root, deployablePath);
            const deploySitePath = this.fsService.dirname(this.fsService.joinPath(resourcesDeployPath, resourceDescriptor.name, deployablePath));

            // Remember, that it goes like so: /a/beef/c.txt -> /a/milk, so it will get copied to /a/milk/c.txt
            copyTasks.push([assetSitePath, deploySitePath]);
            foldersToCreate.add(deploySitePath);
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

          resourceConfig.push(`ensure ${resourceDescriptor.name}`);
        }),
      );
    }

    if (resourceConfig.length) {
      // Create resources.cfg

      const resourceConfigPath = this.fsService.joinPath(resourcesDeployPath, 'resources.cfg');
      const resourceConfigContent = resourceConfig.join('\n');

      await this.fsService.writeFile(resourceConfigPath, resourceConfigContent);
    }
  }

  protected async deployArtifact(buildInfo: ProjectBuildInfo) {
    if (!buildInfo.deployArtifact) {
      return;
    }

    buildInfo.task.setText('Waiting for server artifact to finish updating');
    await this.gameServerManagerService.getUpdateChannelPromise(buildInfo.serverUpdateChannel, ServerUpdateStates.ready);

    buildInfo.task.setText('Deploying server artifact');

    // Copy artifact
    await this.fsService.copyDirContent(this.gameServerManagerService.getServerPath(buildInfo.serverUpdateChannel), buildInfo.artifactDeployPath);

    // Ensure svadhesive enabled for deployed artifact
    await this.gameServerManagerService.ensureSvAdhesiveEnabledAtPath(buildInfo.artifactDeployPath, true);
  }

  protected async deployMiscFiles(project: Project, buildInfo: ProjectBuildInfo) {
    // Ensure variables.cfg
    {
      const varsConfigPath = this.fsService.joinPath(buildInfo.buildPath, 'variables.cfg');
      const varsConfigContent = [
        `# Generated by FxDK`,
        `# DO NOT MODIFY THIS FILE, IT WILL BE OVERWRITTEN ON NEXT PROJECT BUILD`,
        `# You can modify these variables in FxDK`,
        ``,
        `set steam_webApiKey "${buildInfo.steamWebApiKey}"`,
        `sv_tebexSecret "${buildInfo.tebexSecret}"`,
        ``,
        ...this.projectAccess.getInstance().getAssetsConvarCommands(),
        ``
      ];

      await this.fsService.writeFile(varsConfigPath, varsConfigContent.join('\n'));
    }

    // Ensure server.cfg
    {
      const serverConfigPath = this.fsService.joinPath(buildInfo.buildPath, 'server.cfg');
      if (null === (await this.fsService.statSafe(serverConfigPath))) {
        const serverConfigContent = [
          `# Generated by FxDK`,
          `# You can safely modify this file`,
          ``,
          `endpoint_add_tcp "0.0.0.0:30120"`,
          `endpoint_add_udp "0.0.0.0:30120"`,
          ``,
          `# Get your license key at https://keymaster.fivem.net/`,
          `sv_licenseKey REPLACE_WITH_YOUR_LICENSE_KEY`,
          ``,
          `sv_hostname "${project.getName()}"`,
          ``,
          `set onesync on`,
          `sv_maxclients 48`,
          ``,
          `exec variables.cfg`,
          `exec resources/resources.cfg`,
          ``,
        ];

        const blankConfigPath = this.fsService.joinPath(project.getFxserverCwd(), 'blank.cfg');
        if (await this.fsService.statSafe(blankConfigPath)) {
          const blankConfigContent = (await this.fsService.readFileString(blankConfigPath)).trim();
          if (blankConfigContent) {
            serverConfigContent.push('');
            serverConfigContent.push(`# Copied from ${blankConfigPath}`);
            serverConfigContent.push(blankConfigContent);
          }
        }

        await this.fsService.writeFile(serverConfigPath, serverConfigContent.join('\n'));
      }
    }

    // Ensure start.cmd
    if (buildInfo.artifactDeployPathStats) {
      const cmdFilePath = this.fsService.joinPath(buildInfo.buildPath, 'start.cmd');
      if (null === (await this.fsService.statSafe(cmdFilePath))) {
        const cmdFileContent = [
          `@echo off`,
          `find /c "sv_licenseKey REPLACE_WITH_YOUR_LICENSE_KEY" server.cfg >NUL`,
          `if %errorlevel% equ 0 goto :notok`,
          `goto :ok`,
          `:notok`,
          `\techo Please fill out correct license key in server.cfg and restart server`,
          `\tpause`,
          `\tgoto :done`,
          `:ok`,
          `\t.\\artifact\\FXServer.exe +exec server.cfg`,
          `:done`,
        ];

        await this.fsService.writeFile(cmdFilePath, cmdFileContent.join('\n'));
      }
    }
  }

  protected async shouldDeployNewServerArtifact(buildInfo: ProjectBuildInfo): Promise<boolean> {
    const { serverUpdateChannel } = buildInfo;

    const neededArtifactVersion = await this.gameServerManagerService.getInstalledServerVersion(serverUpdateChannel);
    if (!neededArtifactVersion) {
      this.notificationService.warning(`Server artifact won't get deployed as it is not installed`, 10000);
      return false;
    }

    const deployedArtifactVersion = await this.getDeployedArtifactVersion(buildInfo);

    // No need to deploy server artifact if it is already of needed version
    if (neededArtifactVersion === deployedArtifactVersion) {
      this.logService.log('Skipping artifact deployment as it is already deployed and of latest version');
      return false;
    }

    return true;
  }

  protected async getDeployedArtifactVersion(buildInfo: ProjectBuildInfo): Promise<string | null> {
    const deployedVersionFilePath = this.fsService.joinPath(buildInfo.artifactDeployPath, versionFilename);
    const deployedVersionFileStat = await this.fsService.statSafe(deployedVersionFilePath);
    if (!deployedVersionFileStat) {
      return null;
    }

    try {
      const deployedVersionFileContent = await this.fsService.readFileString(deployedVersionFilePath);

      return deployedVersionFileContent?.trim() || null;
    } catch (e) {
      return null;
    }
  }
}
