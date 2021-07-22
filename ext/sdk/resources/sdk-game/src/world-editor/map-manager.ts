import { sendSdkBackendMessage, sendSdkMessage, sendSdkMessageBroadcast } from "../client/sendSdkMessage";
import { joaat } from "../shared";
import { CameraManager } from "./camera-manager";
import {
  WEApplyAdditionChangeRequest,
  WEApplyPatchChangeRequest,
  WECreatePatchRequest,
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
import { SettingsManager } from "./settings-manager";
import { drawDebugText } from "./utils";

export const MapManager = new class MapManager {
  private map: WEMap | null = null;
  private activeMapdatas: number[] = [];
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
    on('we:setAdditionOnGround', (additionId: string) => {
      const addition = this.map.additions[additionId];
      if (!addition) {
        return;
      }

      const handle = this.objects.getObjectHandle(additionId);
      if (typeof handle === 'number') {
        PlaceObjectOnGroundProperly(handle);

        this.updateAddition(additionId, addition, makeEntityMatrix(handle));
      }
    });

    on('we:applyPatchChange', (req: string) => {
      const request: WEApplyPatchChangeRequest = JSON.parse(req);

      if (request.mat) {
        this.handleApplyPatchMatrix(request.mapdata, request.entity, request.mat);
      }
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

      if (SettingsManager.settings.showSelectionBoundingBox) {
        this.drawBoundingBox(selectedEntity);
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
    this.activeMapdatas = [];
  }

  private drawBoundingBox(entity: number) {
    const pos = GetEntityCoords(entity);
    const [_min, _max] = GetModelDimensions(GetEntityModel(entity));

    const min = [_min[0] + pos[0], _min[1] + pos[1], _min[2] + pos[2]];
    const max = [_max[0] + pos[0], _max[1] + pos[1], _max[2] + pos[2]];

    const AF = min;
    // drawTextAt('AF', AF);
    const BF = [min[0], min[1], max[2]];
    // drawTextAt('BF', BF);
    const CF = [min[0], max[1], max[2]];
    // drawTextAt('CF', CF);
    const DF = [min[0], max[1], min[2]];
    // drawTextAt('DF', DF);

    const AB = [max[0], min[1], min[2]];
    // drawTextAt('AB', AB);
    const BB = [max[0], min[1], max[2]];
    // drawTextAt('BB', BB);
    const CB = max;
    // drawTextAt('CB', CB);
    const DB = [max[0], max[1], min[2]];
    // drawTextAt('DB', DB);

    const c = [255, 255, 255, 255];

    // AF -> BF
    drawLine(AF, BF, c);

    // BF -> CF
    drawLine(BF, CF, c);

    // CF -> DF
    drawLine(CF, DF, c);

    // DF -> AF
    drawLine(DF, AF, c);

    // AB -> BB
    drawLine(AB, BB, c);

    // BB -> CB
    drawLine(BB, CB, c);

    // CB -> DB
    drawLine(CB, DB, c);

    // DB -> AB
    drawLine(DB, AB, c);

    // AF -> AB
    drawLine(AF, AB, c);

    // BF -> BB
    drawLine(BF, BB, c);

    // CF -> CB
    drawLine(CF, CB, c);

    // DF -> DB
    drawLine(DF, DB, c);
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
    this.activeMapdatas.push(mapdataHash);

    // if map is not loaded yet - store to pending to load later
    if (this.map === null) {
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
    const index = this.activeMapdatas.indexOf(mapdata);
    if (index > -1) {
      this.activeMapdatas.splice(index, 1);
    }
  }

  private handleApplyPatchMatrix(mapdata: number, entity: number, mat: WEEntityMatrix) {
    const patch = this.map.patches[mapdata]?.[entity];
    if (patch) {
      patch.mat = mat;

      applyMapdataPatch(mapdata, entity, mat);
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

    if (this.activeMapdatas.length) {
      for (const mapdata of this.activeMapdatas) {
        if (this.map.patches[mapdata]) {
          for (const [entity, def] of Object.entries(this.map.patches[mapdata])) {
            applyMapdataPatch(mapdata, parseInt(entity, 10), def.mat);
          }
        }
      }
    }
  }

  private updatePatch(entityGuid: number, mat: Float32Array | WEEntityMatrix) {
    const [success, mapdata, entity] = GetEntityMapdataOwner(entityGuid);
    if (success) {
      applyMapdataPatch(mapdata, entity, mat);

      let patch: WEMapPatch = this.map.patches[mapdata]?.[entity];
      if (patch) {
        patch.mat = prepareEntityMatrix(mat);

        sendSdkMessageBroadcast('we:applyPatchChange', {
          mapdata,
          entity,
          mat: patch.mat,
          cam: CameraManager.getCamLimitedPrecision(),
        } as WEApplyPatchChangeRequest);
      } else {
        patch = {
          label: this.getEntityLabel(entityGuid),
          cam: CameraManager.getCamLimitedPrecision(),
          mat: prepareEntityMatrix(mat),
        };

        this.map.patches[mapdata] ??= {};
        this.map.patches[mapdata][entity] = patch;

        sendSdkMessageBroadcast('we:createPatch', {
          mapDataHash: mapdata,
          entityHash: entity,
          patch,
        } as WECreatePatchRequest);
      }
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
    this.objects.set(additionId, addition, true);

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
  return Array.from(mat) as any;
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

function drawLine(f: [number, number, number] | number[], t: [number, number, number] | number[], c: [number, number, number, number] | number[]) {
  DrawLine(f[0], f[1], f[2], t[0], t[1], t[2], c[0], c[1], c[2], c[3]);
}

function drawTextAt(t: string, c: [number, number, number] | number[]) {
  const [success, x, y] = GetScreenCoordFromWorldCoord(c[0], c[1], c[2]);
  if (success) {
    drawDebugText(t, x, y);
  }
}
