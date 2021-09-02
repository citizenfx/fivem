import { ENABLED_ASSET_MANAGERS } from "assets/enabled-managers";
import { ContributionProvider } from "backend/contribution-provider";
import { inject, injectable, named } from "inversify";
import { FilesystemEntry } from "shared/api.types";
import { AssetType } from "shared/asset.types";
import { AssetManagerContribution } from "./asset/asset-manager-contribution";

@injectable()
export class ProjectAssetManagers {
  @inject(ContributionProvider) @named(AssetManagerContribution)
  protected readonly assetManagerContributions: ContributionProvider<AssetManagerContribution>;

  private managers: Record<AssetType, AssetManagerContribution>;
  private managersValues: AssetManagerContribution[];

  private ensureManagers() {
    if (this.managers) {
      return;
    }

    this.managers = Object.create(null);

    for (const assetType of Object.keys(ENABLED_ASSET_MANAGERS)) {
      try {
        this.managers[assetType] = this.assetManagerContributions.getTagged('assetType', assetType);
      } catch (e) {
        console.error(e);
        return [];
      }
    }

    this.managersValues = Object.values(this.managers);
  }

  get(assetType: AssetType): AssetManagerContribution {
    this.ensureManagers();

    const manager = this.managers[assetType];
    if (!manager) {
      throw new Error(`No manager for asset of type "${assetType}"`);
    }

    return manager;
  }

  getAll(): AssetManagerContribution[] {
    this.ensureManagers();

    return this.managersValues;
  }

  handleFSEntry(entry: FilesystemEntry) {
    this.ensureManagers();

    for (const manager of this.managersValues) {
      manager.handleFSEntry?.(entry);
    }
  }
}
