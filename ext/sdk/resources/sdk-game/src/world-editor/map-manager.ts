import { sendSdkBackendMessage, sendSdkMessage, sendSdkMessageBroadcast } from "../client/sendSdkMessage";
import { joaat, joaatUint32 } from "../shared";
import { CameraManager } from "./camera-manager";
import {
  WEApplyAdditionChangeRequest,
  WEApplyPatchRequest,
  WECreateAdditionRequest,
  WEEntityMatrix,
  WEMap,
  WEMapAddition,
  WEMapPatch,
  WESetAdditionRequest,
} from "./map-types";
import { applyEntityMatrix, limitPrecision, makeEntityMatrix } from "./math";
import { ObjectManager } from "./object-manager";
import { SelectionController } from "./selection-controller";
import { drawDebugText } from "./utils";

export const MapManager = new class MapManager {
  private map: WEMap | null = null;
  private pendingMapdatas: number[] = [];
  private lastCamString: string = '';
  private objects: ObjectManager | null = null;

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

  private updateSelectedEntity() {
    const selectedEntity = SelectionController.getSelectedEntity();

    if (selectedEntity !== null) {
      const entityMatrix = makeEntityMatrix(selectedEntity);

      if (DrawGizmo(entityMatrix as any, selectedEntity.toString())) {
        applyEntityMatrix(selectedEntity, entityMatrix);

        const additionId = this.objects.getObjectId(selectedEntity) || '';
        const addition = this.map.additions[additionId];

        if (addition) {
          this.updateAddition(additionId, addition, entityMatrix);
        } else {
          this.updatePatch(selectedEntity, entityMatrix);
        }
      }
    }
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

  private handleMapdataLoaded(mapdata: number) {
    // if map is not loaded yet - store to pending to load later
    if (this.map === null) {
      this.pendingMapdatas.push(mapdata);
      return;
    }

    const entities = this.map.patches[mapdata];

    if (entities) {
      for (const [entity, def] of Object.entries(entities)) {
        applyMapdataPatch(mapdata, parseInt(entity, 10), def.mat);
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
        label: GetEntityModel(entity).toString(16).toUpperCase(),
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
