import { joaatUint32 } from "../shared";
import { CameraManager } from "./camera-manager";
import { WEEntityMatrixIndex, WEMapAddition } from "@sdk-root/backend/world-editor/world-editor-types";
import { applyAdditionMatrix, applyEntityMatrix } from "./math";
import { Sector, SectorId } from "./sector";
import { Memoizer } from "./utils";

const mdlHashCache = {};
function getAdditionModelHash(mdl: string | number): number {
  if (!(mdl in mdlHashCache)) {
    mdlHashCache[mdl] = typeof mdl === 'string'
      ? joaatUint32(mdl)
      : mdl;
  }
  return mdlHashCache[mdl];
}

function getAdditionSectorId(addition: WEMapAddition): SectorId {
  return Sector.idFromCoords(
    addition.mat[WEEntityMatrixIndex.AX],
    addition.mat[WEEntityMatrixIndex.AY],
  );
}

const HANDLE_NOT_LOADED = Symbol.for('not_loaded');
const HANDLE_LOADING = Symbol.for('loading');

type AdditionId = string;
type Handle =
  | number
  | symbol;

export type ObjectCreatedListener = (additionId: AdditionId, handle: number) => void;
export class ObjectManager {
  private activeHandles: Record<AdditionId, Handle> = {};
  private activeHandlesToAdditionIdMap: Record<Handle, AdditionId> = {};

  private sectors: Record<SectorId, Record<AdditionId, Handle>> = {};
  private activeSectors: SectorId[] = [];
  private activeSectorsMemoizer = new Memoizer(this.activeSectors);
  private additionSectors: Record<AdditionId, SectorId> = {};

  private createEventListeners: ObjectCreatedListener[] = [];

  constructor(
    private additions: Record<AdditionId, WEMapAddition>,
  ) {
    Object.entries(additions).forEach(([additionId, addition]) => {
      this.setAdditionSector(
        additionId,
        getAdditionSectorId(addition),
      );
    });
  }

  destroy() {
    for (const handle of Object.values(this.activeHandles)) {
      if (typeof handle === 'number') {
        DeleteObject(handle);
      }
    }
  }

  onObjectCreated(cb: ObjectCreatedListener) {
    this.createEventListeners.push(cb);
  }

  isAddition(handle: number): boolean {
    return handle in this.activeHandlesToAdditionIdMap;
  }

  getObjectId(handle: number): AdditionId | void {
    return this.activeHandlesToAdditionIdMap[handle];
  }

  getObjectHandle(additionId: AdditionId): Handle | void {
    return this.activeHandles[additionId];
  }

  set(additionId: AdditionId, addition: WEMapAddition, doNotApplyMatrix = false) {
    this.additions[additionId] = addition;

    this.setAdditionSector(
      additionId,
      getAdditionSectorId(addition),
    );

    if (!doNotApplyMatrix) {
      const additionSector = this.additionSectors[additionId];
      const handle = this.activeHandles[additionId];

      if (this.activeSectors.includes(additionSector) && typeof handle === 'number') {
        applyEntityMatrix(handle, addition.mat);
      }
    }
  }

  delete(additionId: AdditionId) {
    const handle = this.activeHandles[additionId];
    delete this.activeHandlesToAdditionIdMap[handle];

    if (typeof handle === 'number') {
      this.deleteObject(handle);
    }

    delete this.additions[additionId];
    delete this.activeHandles[additionId];

    const sectorId = this.additionSectors[additionId];
    if (sectorId) {
      delete this.sectors[sectorId][additionId];
    }
  }

  update() {
    const { x, y } = CameraManager.getPosition();

    const prevActiveSectors = this.activeSectors;
    this.activeSectors = Sector.affectedSectorIdsFromCoords(x, y);

    if (!this.activeSectorsMemoizer.compareAndStore(this.activeSectors)) {
      prevActiveSectors.forEach(this.maybeUnloadSector);
    }

    for (const sectorId of this.activeSectors) {
      this.updateSector(sectorId);
    }
  }

  private updateSector(sectorId: SectorId) {
    const sector = this.sectors[sectorId];
    if (!sector) {
      return;
    }

    for (const additionId in sector) {
      switch (sector[additionId]) {
        case HANDLE_NOT_LOADED: {
          const mdlHash = getAdditionModelHash(this.additions[additionId].mdl);

          RequestModel(mdlHash);
          this.updateHandle(additionId, sectorId, HANDLE_LOADING);
          break;
        }

        case HANDLE_LOADING: {
          const { mdl, mat } = this.additions[additionId];
          const mdlHash = getAdditionModelHash(mdl);

          if (HasModelLoaded(mdlHash)) {
            const handle = CreateObject(mdlHash, mat[12], mat[13], mat[14], false, false, false);

            FreezeEntityPosition(handle, true);

            SetModelAsNoLongerNeeded(mdlHash);

            applyAdditionMatrix(handle, mat);

            this.updateHandle(additionId, sectorId, handle);

            this.createEventListeners.forEach((cb) => cb(additionId, handle));
          }
          break;
        }
      }
    }
  }

  private updateHandle(additionId: AdditionId, sectorId: SectorId, handle: Handle) {
    if (handle === HANDLE_NOT_LOADED) {
      const prevHandle = this.activeHandles[additionId];

      if (typeof prevHandle === 'number') {
        delete this.activeHandles[additionId];
        delete this.activeHandlesToAdditionIdMap[prevHandle];
      }
    }

    if (typeof handle === 'number') {
      this.activeHandles[additionId] = handle;
      this.activeHandlesToAdditionIdMap[handle] = additionId;
    }

    this.sectors[sectorId][additionId] = handle;
  }

  private maybeUnloadSector = (sectorId: SectorId) => {
    const sector = this.sectors[sectorId];
    if (!sector) {
      return;
    }

    if (!this.activeSectors.includes(sectorId)) {
      for (const additionId in sector) {
        const handle = sector[additionId];

        if (typeof handle !== 'symbol') {
          this.deleteObject(handle);
        }

        this.updateHandle(additionId, sectorId, HANDLE_NOT_LOADED);
      }
    }
  };

  private setAdditionSector(additionId: AdditionId, sectorId: SectorId) {
    const prevSectorId = this.additionSectors[additionId];

    let handle: Handle = HANDLE_NOT_LOADED;

    this.additionSectors[additionId] = sectorId;

    if (prevSectorId !== undefined) {
      handle = this.sectors[prevSectorId][additionId];

      delete this.sectors[prevSectorId][additionId];

      if (Object.keys(this.sectors[prevSectorId]).length === 0) {
        delete this.sectors[prevSectorId];
      }
    }

    if (!this.sectors[sectorId]) {
      this.sectors[sectorId] = {};
    }

    this.updateHandle(additionId, sectorId, handle);
  }

  private deleteObject(handle: number) {
    if (DoesEntityExist(handle)) {
      DeleteObject(handle);
    }
  }
}
