import { injectable } from "inversify";
import { ProjectInstanceService } from "./project";

@injectable()
export class ProjectAccess {
  private project: ProjectInstanceService | null = null;

  setInstance(project: ProjectInstanceService | null) {
    this.project = project;
  }

  hasInstance(): boolean {
    return this.project !== null;
  }

  getInstance(): ProjectInstanceService {
    if (!this.hasInstance()) {
      throw new Error('No project');
    }

    return this.project!;
  }

  async withInstance<T>(cb: (project: ProjectInstanceService) => Promise<T> | T): Promise<T | void> {
    if (!this.hasInstance()) {
      return;
    }

    return await cb(this.project!);
  }
}
