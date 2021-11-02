import { Project } from "../project";

export class AssetRuntimeData<T> {
  constructor(private assetPath: string) {}

  get(): T | undefined {
    return Project.assetRuntimeData.get<T>(this.assetPath);
  }
}
