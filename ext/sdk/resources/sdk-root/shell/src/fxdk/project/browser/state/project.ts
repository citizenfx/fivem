import { dispose } from "fxdk/base/disposable";
import { ProjectInstance } from "./projectInstance";

let instance: ProjectInstance | null = null;

function projectGuard() {
  if (!instance) {
    throw new Error('No project open yet');
  }
}

export const Project = new Proxy((({} as any) as ProjectInstance), {
  get(_target, property) {
    projectGuard();

    return instance![property];
  }
});

export function setProjectInstance(projectInstance: ProjectInstance) {
  instance = projectInstance;
}

export async function disposeProjectInstance() {
  if (!instance) {
    return;
  }

  dispose(instance);

  instance = null;
}
