import { ProjectParticipants } from "fxdk/project/browser/projectExtensions";
import { Project } from "fxdk/project/browser/state/project";
import { ConvarKind, IConvarCategoryMap, IConvarEntry, ProjectVariableSetter } from "fxdk/project/common/project.types";
import { computed } from "mobx";

export function convarEntryKindToJsType(entry: IConvarEntry): 'string' | 'number' | 'boolean' {
  switch (entry.kind) {
    case ConvarKind.Bool: return 'boolean';

    case ConvarKind.Combi:
    case ConvarKind.Int:
    case ConvarKind.Slider: return 'number';

    case ConvarKind.String:
    case ConvarKind.Password:
    case ConvarKind.Multi: return 'string';
  }
}

// `${setter}:${kind}`
export type ConvarDefinitionId = string;
export type Conflict = Record<ConvarDefinitionId, Array<{
  categoryName: string,
  entry: IConvarEntry,
}>>;

export const computeVariables = () => computed(() => {
  const categoriesMap: IConvarCategoryMap = {};
  const typeConflicts: Record<string, Conflict> = {};

  for (const provider of ProjectParticipants.getAllVariablesProviders()) {
    for (const [categoryName, category] of Object.entries(provider.getConvarCategories())) {
      const categoryFullyQualifiedName = `${provider.label}/${categoryName}`;

      categoriesMap[categoryFullyQualifiedName] = category;

      for (const entry of category.entries) {
        const convarDefId = `${entry.setter}:${entry.kind}`;

        typeConflicts[entry.variable] ??= {};
        typeConflicts[entry.variable][convarDefId] ??= [];
        typeConflicts[entry.variable][convarDefId].push({
          categoryName: categoryFullyQualifiedName,
          entry,
        });
      }
    }
  }

  const projectVariables = Project.manifest.variables || {};
  const boundVariableNames = Object.keys(typeConflicts);

  return {
    categoriesMap,
    typeConflicts: Object.entries(typeConflicts).reduce((conflicts, [variable, conflict]) => {
      if (Object.keys(conflict).length > 1) {
        conflicts[variable] = conflict;
      }

      return conflicts;
    }, {}) as Record<string, Conflict>,
    strandedVariables: Object.fromEntries(
      Object.keys(projectVariables)
        .filter((variable) => !boundVariableNames.includes(variable))
        .map((variable) => [variable, projectVariables[variable]])
    ),
  };
}).get();
