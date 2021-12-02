import { IConvarCategoryMap } from "fxdk/project/common/project.types";

export interface IResourceRuntimeData {
  ready: boolean,
  convarCategories?: IConvarCategoryMap,
}
