import { ServerResourceDescriptor, ServerVariableDescriptor } from "./game-server-runtime";

export interface ReconciledServerResources {
  load: ServerResourceDescriptor[],
  unload: ServerResourceDescriptor[],
}

export interface ReconciledServerVariables {
  set: ServerVariableDescriptor[],
  unset: ServerVariableDescriptor[],
}

type ResourceName = string;
type ResourcePath = string;
type ResourcesMap = Map<ResourceName, ResourcePath>;
function resourcesToMap(resources: ServerResourceDescriptor[]): ResourcesMap {
  return new Map(resources.map(({ name, path }) => [name, path]));
}

type VariableName = string;
type VariablesMap = Map<VariableName, ServerVariableDescriptor>;
function variablesToMap(variables: ServerVariableDescriptor[]): VariablesMap {
  return new Map(variables.map((variable) => [variable.name, variable]));
}

export class GameServerResourcesReconciler {
  private currentResources: ResourcesMap = new Map();
  private currentVariables: VariablesMap = new Map();

  setInitialResources(resources: ServerResourceDescriptor[]) {
    this.currentResources = resourcesToMap(resources);
  }

  setResources(resources: ServerResourceDescriptor[]): ReconciledServerResources {
    const resourcesMap = resourcesToMap(resources);

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

      // paths are different for the same resource name - load new and unload old
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

  setInitialVariables(variables: ServerVariableDescriptor[]) {
    this.currentVariables = variablesToMap(variables);
  }

  setVariables(variables: ServerVariableDescriptor[]): ReconciledServerVariables {
    const variablesMap = variablesToMap(variables);

    const reconciled: ReconciledServerVariables = {
      set: [],
      unset: [],
    };

    const visited = new Set<string>();

    for (const variable of variables) {
      visited.add(variable.name);

      if (hasVariableAddedOrChanged(variable, this.currentVariables)) {
        reconciled.set.push(variable);
      }
    }

    for (const variable of this.currentVariables.values()) {
      if (visited.has(variable.name)) {
        continue;
      }

      reconciled.unset.push(variable);
    }

    this.currentVariables = variablesMap;

    return reconciled;
  }
}

function hasVariableAddedOrChanged(variable: ServerVariableDescriptor, currentVariables: VariablesMap): boolean {
  const currentVariable = currentVariables.get(variable.name);
  if (!currentVariable) {
    return true;
  }

  if (currentVariable.value !== variable.value) {
    return true;
  }

  if (currentVariable.setter !== variable.setter) {
    return true;
  }

  return false;
}
