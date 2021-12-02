import { handlesClientEvent } from "backend/api/api-decorators";
import { FsService } from "backend/fs/fs-service";
import { NotificationService } from "backend/notification/notification-service";
import { ProjectAccess } from "fxdk/project/node/project-access";
import { SystemResourcesService } from "backend/system-resources/system-resources-service";
import { Task, TaskReporterService } from "backend/task/task-reporter-service";
import { inject, injectable } from "inversify";
import { ExamplesImporterApi } from "../common/importer.examples";

@injectable()
export class ExamplesImporter {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @inject(SystemResourcesService)
  protected readonly systemResourcesService: SystemResourcesService;

  @handlesClientEvent(ExamplesImporterApi.Endpoints.import)
  async importAsset(request: ExamplesImporterApi.ImportRequest): Promise<boolean> {
    const importTask = this.taskReporterService.create(`Importing example`);

    try {
      await this.doImport(importTask, request);
      this.notificationService.info(`Succefully imported ${request.exampleName} example`, 5000);
      return true;
    } catch (e) {
      this.notificationService.error(`Failed to import example: ${e.toString()}`);
      return false;
    } finally {
      importTask.done();
    }
  }

  private async doImport(task: Task, request: ExamplesImporterApi.ImportRequest) {
    const { exampleName } = request;

    const examplePath = this.systemResourcesService.getResourcePath(exampleName);
    if (!(await this.fsService.statSafe(examplePath))) {
      throw new Error(`Unable to locate example at ${examplePath}`);
    }

    const assetPath = this.fsService.joinPath(request.basePath, request.name);

    await this.fsService.mkdirp(assetPath);

    await this.fsService.copyDirContent(
      examplePath,
      assetPath,
      {
        onProgress(progress) {
          task.setProgress(progress);
        },
      },
    );
  }
}
