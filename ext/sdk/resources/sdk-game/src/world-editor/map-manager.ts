import { sendSdkBackendMessage, sendSdkMessage, sendSdkMessageBroadcast } from "../client/sendSdkMessage";
import { joaat, joaatUint32 } from "../shared";
import { CameraManager } from "./camera-manager";
import {
  WEApplyAdditionChangeRequest,
  WEApplyPatchRequest,
  WECreateAdditionRequest,
  WEDeletePatchRequest,
  WEEntityMatrix,
  WEMap,
  WEMapAddition,
  WEMapPatch,
  WESelectionType,
  WESetAdditionRequest,
  WESetSelectionRequest,
} from "./map-types";
import { applyEntityMatrix, limitPrecision, makeEntityMatrix } from "./math";
import { ObjectManager } from "./object-manager";
import { SelectionController } from "./selection-controller";

export const MapManager = new class MapManager {
  private map: WEMap | null = null;
  private pendingMapdatas: number[] = [];
  private lastCamString: string = '';
  private objects: ObjectManager | null = null;

  private patchBackupMatrices: Record<string, Float32Array> = {};

  preinit() {
    EnableEditorRuntime();

    on('mapDataLoaded', (mapdata: number) => this.handleMapdataLoaded(mapdata));
    on('mapDataUnloaded', (mapdata: number) => this.handleMapdataUnloaded(mapdata));

    on('we:createAddition', (request: string) => {
      this.handleSpawnObject(JSON.parse(request));
    });
    on('we:setAddition', (req: string) => {
      const [additionId, addition] = JSON.parse(req);

      if (this.map) {
        this.map.additions[additionId] = addition;
        this.objects.set(additionId, addition);
      }
    });
    on('we:applyAdditionChange', (req: string) => {
      const request: WEApplyAdditionChangeRequest = JSON.parse(req);

      this.handleApplyAdditionChange(request);
    });

    on('we:deletePatch', (req: string) => {
      const request: WEDeletePatchRequest = JSON.parse(req);

      this.handleDeletePatch(request.mapDataHash, request.entityHash);
    });

    on('we:deleteAddition', (objectId: string) => {
      this.handleDeleteAddition(objectId);
    });
    on('we:deleteAdditions', (ids: string) => {
      this.handleDeleteAddition(JSON.parse(ids));
    });

    on('we:map', (mapString: string) => {
      this.map = JSON.parse(mapString);

      setTimeout(() => {
        this.applyLoadedMap();

        sendSdkMessage('we:ready');
      }, 0);
    });

    on('we:setCam', (camString: string) => {
      CameraManager.setCam(JSON.parse(camString));
    });
  }

  init() {
    sendSdkBackendMessage('we:accept');
  }

  update() {
    if (!this.map) {
      return;
    }

    if (this.objects) {
      this.objects.update();
    }

    const cam = CameraManager.getCamLimitedPrecision();
    const camString = JSON.stringify(cam);
    if (camString !== this.lastCamString) {
      this.lastCamString = camString;
      sendSdkBackendMessage('we:setCam', cam);
    }

    this.updateSelectedEntity();
  }

  private prevSelectedEntity = null;
  private updateSelectedEntity() {
    const selectedEntity = SelectionController.getSelectedEntity();
    const hasSelectedEntityChanged = selectedEntity !== this.prevSelectedEntity;

    if (selectedEntity !== null) {
      const entityMatrix = makeEntityMatrix(selectedEntity);

      const additionId = this.objects.getObjectId(selectedEntity) || '';
      const addition = this.map.additions[additionId];

      if (hasSelectedEntityChanged) {
        if (addition) {
          sendSdkMessage('we:setSelection', {
            type: WESelectionType.ADDITION,
            id: additionId,
            label: this.getEntityLabel(selectedEntity),
          } as WESetSelectionRequest);
        } else {
          const [success, mapdataHash, entityHash] = GetEntityMapdataOwner(selectedEntity);
          if (success) {
            // Save initial patch matrix allowing its deletion
            const key = getPatchKey(mapdataHash, entityHash);
            if (!this.patchBackupMatrices[key]) {
              this.patchBackupMatrices[key] = entityMatrix;
            }

            sendSdkMessage('we:setSelection', {
              type: WESelectionType.PATCH,
              mapdata: mapdataHash,
              entity: entityHash,
              label: this.getEntityLabel(selectedEntity),
            } as WESetSelectionRequest);
          }
        }
      }

      // Save initial patch matrix
      if (!addition && hasSelectedEntityChanged) {
        const [success, mapdataHash, entityHash] = GetEntityMapdataOwner(selectedEntity);
        if (success) {
          const key = getPatchKey(mapdataHash, entityHash);
          if (!this.patchBackupMatrices[key]) {
            this.patchBackupMatrices[key] = entityMatrix;
          }
        }
      }

      if (DrawGizmo(entityMatrix as any, selectedEntity.toString())) {
        applyEntityMatrix(selectedEntity, entityMatrix);

        if (addition) {
          this.updateAddition(additionId, addition, entityMatrix);
        } else {
          this.updatePatch(selectedEntity, entityMatrix);
        }
      }
    } else {
      if (hasSelectedEntityChanged) {
        sendSdkMessage('we:setSelection', {
          type: WESelectionType.NONE,
        } as WESetSelectionRequest);
      }
    }

    this.prevSelectedEntity = selectedEntity;
  }

  destroy() {
    Citizen.invokeNative(joaat('DISABLE_EDITOR_RUNTIME'));

    this.map = null;
    this.objects = null;
    this.pendingMapdatas = [];
  }

  private handleSpawnObject(request: WECreateAdditionRequest) {
    if (request.needsPlacement) {
      const pos = getObjectPosition();

      request.object.cam = CameraManager.getCamLimitedPrecision();
      request.object.mat = prepareEntityMatrix([
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        pos.x, pos.y, pos.z, 1,
      ]);

      sendSdkMessageBroadcast('we:setAddition', {
        id: request.id,
        object: request.object,
      } as WESetAdditionRequest);
    }

    this.map.additions[request.id] = request.object;
    this.objects.set(request.id, request.object);
  }

  handleDeletePatch(mapdataHash: number, entityHash: number) {
    const key = getPatchKey(mapdataHash, entityHash);

    if (this.patchBackupMatrices[key]) {
      applyMapdataPatch(mapdataHash, entityHash, this.patchBackupMatrices[key]);
      delete this.patchBackupMatrices[key];
    }

    if (this.map.patches[mapdataHash]) {
      delete this.map.patches[mapdataHash][entityHash];
    }
  }

  private handleDeleteAddition(objectId: string) {
    this.handleDeleteAdditions([objectId]);
  }

  private handleDeleteAdditions(ids: string[]) {
    if (this.map) {
      for (const id of ids) {
        delete this.map.additions[id];
        this.objects.delete(id);
      }
    }
  }

  private handleMapdataLoaded(mapdataHash: number) {
    // if map is not loaded yet - store to pending to load later
    if (this.map === null) {
      this.pendingMapdatas.push(mapdataHash);
      return;
    }

    const entities = this.map.patches[mapdataHash];

    if (entities) {
      for (const [entityHashString, def] of Object.entries(entities)) {
        const key = getPatchKey(mapdataHash, entityHashString);
        const entityHash = parseInt(entityHashString, 10);

        if (!this.patchBackupMatrices[key]) {
          this.patchBackupMatrices[key] = getMapdataEntityMatrix(mapdataHash, entityHash);
        }

        applyMapdataPatch(mapdataHash, entityHash, def.mat);
      }
    }
  }

  private handleMapdataUnloaded(mapdata: number) {
    // If map is loaded - don't care about unloaded mapdata
    if (this.map !== null) {
      return;
    }

    const index = this.pendingMapdatas.indexOf(mapdata);
    if (index > -1) {
      this.pendingMapdatas.splice(index, 1);
    }
  }

  private handleApplyAdditionChange(request: WEApplyAdditionChangeRequest) {
    if (this.map === null) {
      return;
    }

    const additionId = request.id;
    delete request['id'];
    const addition = this.map.additions[additionId];
    if (addition) {
      const newAddition = {
        ...addition,
        ...request,
      };

      this.map.additions[additionId] = newAddition;
      this.objects.set(additionId, newAddition);
    }
  }

  private applyLoadedMap() {
    CameraManager.setCam(this.map.meta.cam);

    this.objects = new ObjectManager(this.map.additions);

    if (this.pendingMapdatas.length) {
      for (const mapdata of this.pendingMapdatas) {
        if (this.map.patches[mapdata]) {
          for (const [entity, def] of Object.entries(this.map.patches[mapdata])) {
            applyMapdataPatch(mapdata, parseInt(entity, 10), def.mat);
          }
        }
      }
    }
  }

  private updatePatch(entity: number, mat: Float32Array | WEEntityMatrix) {
    const [success, mapdataHash, entityHash] = GetEntityMapdataOwner(entity);
    if (success) {
      const patch: WEMapPatch = {
        label: this.getEntityLabel(entity),
        cam: CameraManager.getCamLimitedPrecision(),
        mat: prepareEntityMatrix(mat),
      };

      this.map.patches[mapdataHash] ??= {};
      this.map.patches[mapdataHash][entityHash] = patch;

      applyMapdataPatch(mapdataHash, entityHash, mat);

      sendSdkMessageBroadcast('we:applyPatch', {
        mapDataHash: mapdataHash,
        entityHash: entityHash,
        patch,
      } as WEApplyPatchRequest);
    }
  }

  private updateAddition(additionId: string, addition: WEMapAddition, mat: Float32Array | WEEntityMatrix) {
    const change: WEApplyAdditionChangeRequest = {
      id: additionId,
      cam: CameraManager.getCamLimitedPrecision(),
      mat: prepareEntityMatrix(mat),
    };

    addition.cam = change.cam;
    addition.mat = change.mat;

    this.map.additions[additionId] = addition;
    this.objects.set(additionId, addition);

    sendSdkMessageBroadcast('we:applyAdditionChange', change);
  }

  private getEntityLabel(entity: number): string {
    return GetEntityArchetypeName(entity) || GetEntityModel(entity).toString(16).toUpperCase();
  }
};

function applyMapdataPatch(mapdata: number, entity: number, mat: Float32Array | number[]) {
  const md = GetMapdataFromHashKey(mapdata);
  const ent = GetEntityIndexFromMapdata(md, entity);

  if (ent === -1) {
    console.error('Failed to get entity index from mapdata', {
      mapdataHash: mapdata,
      mapdataIndex: md,
      entityHash: entity,
    });
    return;
  }

  UpdateMapdataEntity(md, ent, {
    position: [mat[12], mat[13], mat[14]],
    matrix: Array.from(mat),
  });
}

function getObjectPosition() {
  const fw = CameraManager.getForwardVector().copy().mult(5);

  return CameraManager.getPosition().copy().add(fw);
}

function prepareEntityMatrix(mat: Float32Array | WEEntityMatrix): WEEntityMatrix {
  return limitPrecision(Array.from(mat), 10000) as any;
}

const GET_MAPDATA_ENTITY_MATRIX = joaat('GET_MAPDATA_ENTITY_MATRIX');
function getMapdataEntityMatrix(mapdata: number, entity: number): Float32Array {
  const matrix = new Float32Array(16);

  const success: boolean = Citizen.invokeNative(
    GET_MAPDATA_ENTITY_MATRIX,
    mapdata,
    entity,
    matrix,
    Citizen.returnResultAnyway(),
  );

  if (!success) {
    console.error('Failed to get mapdata entity matrix', { mapdata, entity });
  }

  return matrix;
}

function getPatchKey(mapdata: string | number, entity: string | number): string {
  return `${mapdata}:${entity}`;
}
