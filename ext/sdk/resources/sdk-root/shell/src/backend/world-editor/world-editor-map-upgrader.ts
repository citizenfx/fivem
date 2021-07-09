import { LogService } from "backend/logger/log-service";
import deepmerge from "deepmerge";
import { inject, injectable } from "inversify";
import { fastRandomId } from "utils/random";
import { WORLD_EDITOR_MAP_NO_GROUP } from "./world-editor-constants";
import { WEMap, WEMapVersion } from "./world-editor-types";

@injectable()
export class WorldEditorMapUpgrader {
  static CURRENT_VERSION = WEMapVersion.V3;

  @inject(LogService)
  protected readonly logService: LogService;

  async upgrade(map: WEMap): Promise<WEMap> {
    if (map.version === WorldEditorMapUpgrader.CURRENT_VERSION) {
      this.logService.log('Map is already of latest version, skipping upgrade');
      return map;
    }

    let upgradedMap = map;
    let upgrader = this.upgraders[map.version];

    while (upgrader) {
      upgradedMap = await upgrader(upgradedMap);
      upgrader = this.upgraders[upgradedMap.version];

      this.logService.log('Upgraded map to version', upgradedMap.version);
    }

    return upgradedMap;
  }

  private upgradeV1toV2 = (map: WEMap): WEMap => {
    const upgradedMap = deepmerge({}, { ...map });
    upgradedMap.version = WEMapVersion.V2;

    const additionGroups: string[] = map.additionGroups as any;

    if (!Array.isArray(additionGroups)) {
      upgradedMap.additionGroups = {};

      for (const addition of Object.values(upgradedMap.additions)) {
        addition.grp = WORLD_EDITOR_MAP_NO_GROUP;
      }

      return upgradedMap;
    }

    const additionGroupsIds = additionGroups.map(() => fastRandomId());

    upgradedMap.additionGroups = additionGroups.reduce((acc, label, index) => {
      acc[additionGroupsIds[index]] = { label };

      return acc;
    }, {});

    for (const addition of Object.values(upgradedMap.additions)) {
      if (addition.grp === WORLD_EDITOR_MAP_NO_GROUP) {
        continue;
      }

      addition.grp = additionGroupsIds[addition.grp];
    }

    return upgradedMap;
  };

  private upgradeV2toV3 = (map: WEMap): WEMap => {
    const upgradedMap = deepmerge({}, { ...map });
    upgradedMap.version = WEMapVersion.V3;

    for (const addition of Object.values(upgradedMap.additions)) {
      addition.mdl = (addition as any).hash;
      delete (addition as any).hash;
    }

    return upgradedMap;
  };

  private upgraders = {
    [WEMapVersion.V1]: this.upgradeV1toV2,
    [WEMapVersion.V2]: this.upgradeV2toV3,
  };
}
