import { FsService } from "backend/fs/fs-service";
import { NotificationService } from "backend/notification/notification-service";
import { ProjectAccess } from "backend/project/project-access";
import { SystemResourcesService } from "backend/system-resources/system-resources-service";
import { Task, TaskReporterService } from "backend/task/task-reporter-service";
import { inject, injectable } from "inversify";
import { AssetImporterContribution } from "../../asset-importer-contribution";
import { ExampleAssetImportRequest } from "./example-importer.types";

@injectable()
export class ExampleImporter implements AssetImporterContribution {
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

  async importAsset(request: ExampleAssetImportRequest): Promise<boolean> {
    const importTask = this.taskReporterService.create(`Importing example`);

    try {
      await this.doImport(importTask, request);
      this.notificationService.info(`Succefully imported ${request.data?.exampleName} example`, 5000);
      return true;
    } catch (e) {
      this.notificationService.error(`Failed to import example: ${e.toString()}`);
      return false;
    } finally {
      importTask.done();
    }
  }

  private async doImport(task: Task, request: ExampleAssetImportRequest) {
    const exampleName = request.data?.exampleName;
    if (!exampleName) {
      throw new Error('No example name specified');
    }

    const examplePath = this.systemResourcesService.getResourcePath(exampleName);
    if (!(await this.fsService.statSafe(examplePath))) {
      throw new Error(`Unable to locate example at ${examplePath}`);
    }

    const assetPath = this.fsService.joinPath(request.assetBasePath, request.assetName);

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
