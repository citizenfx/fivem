import { ServerResourceDescriptor } from "./game-server-runtime";

export interface ReconciledServerResources {
  load: ServerResourceDescriptor[],
  unload: ServerResourceDescriptor[],
}

type ResourceName = string;
type ResourcePath = string;

type ResourcesMap = Map<ResourceName, ResourcePath>;

function toMap(resources: ServerResourceDescriptor[]): ResourcesMap {
  return new Map(resources.map(({ name, path }) => [name, path]));
}

export class GameServerResourcesReconciler {
  private currentResources: ResourcesMap = new Map();

  setInitialResources(resources: ServerResourceDescriptor[]) {
    this.currentResources = toMap(resources);
  }

  setResources(resources: ServerResourceDescriptor[]): ReconciledServerResources {
    const resourcesMap = toMap(resources);

    const reconciled: ReconciledServerResources = {
      load: [],
      unload: [],
    };

    const visited = new Set<string>();

    for (const [name, path] of resourcesMap.entries()) {
      visited.add(name);

      const currentPath = this.currentResources.get(name);

      // resource intact
      if (currentPath === path) {
        continue;
      }

      // if no current resource - load
      if (!currentPath) {
        reconciled.load.push({ name, path });
        continue;
      }

      // paths are different ofr the same resource name - load new and unload old
      reconciled.load.push({ name, path });
      reconciled.unload.push({ name, path: currentPath });
    }

    for (const [name, path] of this.currentResources.entries()) {
      if (visited.has(name)) {
         continue;
      }

      reconciled.unload.push({ name, path });
    }

    this.currentResources = resourcesMap;

    return reconciled;
  }
}
