import { WEApi } from "@sdk-root/backend/world-editor/world-editor-game-api";
import { WEApplyAdditionChangeRequest, WEEntityMatrix, WEMapAddition } from "@sdk-root/backend/world-editor/world-editor-types";
import { Addition } from "./addition";
import { CameraManager } from "./camera-manager";
import { makeEntityMatrix } from "./math";
import { Sector, SectorId } from "./sector";
import { invokeWEApiBroadcast } from "./utils";

export class AdditionsManager {
  private additions: Record<string, Addition> = {};

  private handlesMap: Record<number, Addition> = {};

  private sectorsMap: Record<SectorId, Record<string, Addition>> = {};

  private sectorManager = new SectorManager();

  constructor(
    additions: Record<string, WEMapAddition>,
    private readonly handleCreated: (additionId: string, handle: number) => void,
  ) {
    Object.entries(additions).forEach(([additionId, data]) => {
      this.create(additionId, data);
    });
  }

  has(additionId: string): boolean {
    return !!this.additions[additionId];
  }

  getHandleFromAddition(additionId: string): number | undefined {
    return this.additions[additionId]?.handle;
  }

  getAdditionIdFromHandle(handle: number): string | undefined {
    return this.handlesMap[handle]?.id;
  }

  create(additionId: string, data: WEMapAddition) {
    const addition = new Addition(additionId, data.mdl, data.mat);

    this.additions[additionId] = addition;

    this.sectorsMap[addition.sectorId] ??= {};
    this.sectorsMap[addition.sectorId][addition.id] = addition;
  }

  getHandle(additionId: string): number | undefined {
    return this.handlesMap[additionId];
  }

  update() {
    this.updateSectors();
  }

  dispose() {
    for (const addition of Object.values(this.additions)) {
      addition.dispose();
    }
  }

  delete(additionId: string) {
    const addition = this.additions[additionId];
    if (!addition) {
      return;
    }

    addition.dispose();

    delete this.additions[additionId];
    delete this.handlesMap[additionId];

    if (this.sectorsMap[addition.sectorId]) {
      delete this.sectorsMap[addition.sectorId][additionId];
    }
  }

  updateAdditionFromShell(additionId: string, matrix: WEEntityMatrix) {
    const addition = this.additions[additionId];
    if (!addition) {
      return;
    }

    this.updateAdditionMatrix(addition, matrix);
  }

  updateAdditionFromGizmo(additionId: string, rawMatrix: Float32Array) {
    const addition = this.additions[additionId];
    if (!addition) {
      return;
    }

    const matrix = Array.from(rawMatrix) as WEEntityMatrix;

    this.updateAdditionMatrix(addition, matrix);

    invokeWEApiBroadcast(WEApi.AdditionApplyChange, {
      id: additionId,
      cam: CameraManager.getCamLimitedPrecision(),
      mat: matrix,
    } as WEApplyAdditionChangeRequest);
  }

  setAdditionOnGround(additionId: string) {
    const handle = this.handlesMap[additionId];
    if (handle !== undefined) {
      PlaceObjectOnGroundProperly(handle);

      this.updateAdditionFromGizmo(additionId, makeEntityMatrix(handle));
    }
  }

  private updateAdditionMatrix(addition: Addition, matrix: WEEntityMatrix) {
    const oldSectorId = addition.sectorId;
    addition.updateMatrix(matrix);
    const newSectorId = addition.sectorId;

    if (oldSectorId !== newSectorId) {
      this.updateAdditionSector(oldSectorId, newSectorId, addition);
    }
  }

  private updateAdditionSector(oldId: SectorId, newId: SectorId, addition: Addition) {
    delete this.sectorsMap[oldId][addition.id];

    this.sectorsMap[newId] ??= {};
    this.sectorsMap[newId][addition.id] = addition;
  }

  private updateSectors() {
    for (const [sectorId, active] of this.sectorManager.getSectors()) {
      if (active) {
        this.updateSector(sectorId);
      } else {
        this.unloadSector(sectorId);
      }
    }
  }

  private updateSector(sectorId: string) {
    const sector: Record<SectorId, Addition> = this.sectorsMap[sectorId];
    if (!sector) {
      return;
    }

    const sectorAdditions = Object.values(sector);
    if (!sectorAdditions.length) {
      return;
    }

    for (const addition of sectorAdditions) {
      if (addition.isNotLoaded) {
        addition.load();
      } else {
        addition.update();
      }

      if (addition.handle !== undefined && this.handlesMap[addition.id] === undefined) {
        this.handlesMap[addition.handle] = addition;
        this.handleCreated(addition.id, addition.handle);
      }
    }
  }

  private unloadSector(sectorId: string) {
    const sector: Record<SectorId, Addition> = this.sectorsMap[sectorId];
    if (!sector) {
      return;
    }

    const sectorAdditions = Object.values(sector);
    if (!sectorAdditions.length) {
      return;
    }

    for (const addition of sectorAdditions) {
      addition.unload();
      delete this.handlesMap[addition.handle];
    }
  }
}

class SectorManager {
  private activeSectors: Record<SectorId, true> = {};

  *getSectors() {
    const { x, y } = CameraManager.getPosition();

    const lastActiveSectors = this.activeSectors;
    this.activeSectors = Sector.affectedSectorIdsFromCoords(x, y);

    const allSectors = Object.assign(lastActiveSectors, this.activeSectors);

    for (const sectorId of Object.keys(allSectors)) {
      yield ([sectorId, !!this.activeSectors[sectorId]] as [string, boolean]);
    }
  }
}
