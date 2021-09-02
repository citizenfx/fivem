import { injectable } from "inversify";
import { Project } from "./project";

@injectable()
export class ProjectAccess {
  private project: Project | null = null;

  setInstance(project: Project | null) {
    this.project = project;
  }

  hasInstance(): boolean {
    return this.project !== null;
  }

  getInstance(): Project {
    if (!this.hasInstance()) {
      throw new Error('No project');
    }

    return this.project!;
  }

  async withInstance<T>(cb: (project: Project) => Promise<T> | T): Promise<T | void> {
    if (!this.hasInstance()) {
      return;
    }

    return await cb(this.project!);
  }
}
