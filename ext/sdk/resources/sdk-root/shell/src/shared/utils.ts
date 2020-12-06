import { ProjectData, ProjectManifest, ProjectManifestResource, ProjectResources } from 'shared/api.types';

const resourceDefault: ProjectManifestResource = {
  name: '',
  enabled: false,
  restartOnChange: false,
};

export const debounce = <T extends Function>(fn: T, timeout: number): T => {
  // any as should be suitable both for node and browser
  let timer: any = null;

  const newFn = (...args) => {
    if (timer) {
      clearTimeout(timer);
    }

    timer = setTimeout(() => {
      timer = null;

      fn(...args);
    }, timeout);
  };

  return newFn as any;
};

export function notNull<T>(e: T | null): e is T {
  return !!e;
}

export function getResourceConfig(manifest: ProjectManifest | void, resourceName: string): ProjectManifestResource {
  const defaults = {
    ...resourceDefault,
    name: resourceName,
  };

  if (!manifest) {
    return defaults;
  }

  return manifest.resources[resourceName] || {
    ...resourceDefault,
    name: resourceName,
  };
}

export const getProjectResources = (project: ProjectData): ProjectResources => {
  const entries = new Set(project.fsTree.entries);
  const resources: ProjectResources = {};

  for (const entry of entries) {
    if (entry.name[0] === '.') {
      continue;
    }

    if (entry.meta.isResource) {
      resources[entry.path] = {
        ...getResourceConfig(project.manifest, entry.name),
        path: entry.path,
        running: false,
      };

      continue;
    }

    if (entry.isDirectory) {
      const childEntries = project.fsTree.pathsMap[entry.path] || [];

      for (const childEntry of childEntries) {
        entries.add(childEntry);
      }
    }
  }

  return resources;
};

export const getEnabledResourcesPaths = (project: ProjectData, projectResources: ProjectResources) => {
  return Object.values(projectResources)
    .filter((resource) => getResourceConfig(project.manifest, resource.name).enabled)
    .map((resource) => resource.path);
};
