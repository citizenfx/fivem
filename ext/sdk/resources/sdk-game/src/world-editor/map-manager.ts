import { sendSdkBackendMessage, sendSdkMessage } from "../client/sendSdkMessage";
import { joaat } from "../shared";
import { CameraManager } from "./camera-manager";
import { WorldEditorEntityMatrix, WorldEditorMap } from "./map-types";
import { applyEntityMatrix, limitPrecision, makeEntityMatrix } from "./math";
import { SelectionController } from "./selection-controller";

export const MapManager = new class MapManager {
  private map: WorldEditorMap | null = null;
  private pendingMapdatas: number[] = [];
  private lastCamString: string = '';

  preinit() {
    EnableEditorRuntime();

    on('mapDataLoaded', (mapdata: number) => this.handleMapdataLoaded(mapdata));
    on('mapDataUnloaded', (mapdata: number) => this.handleMapdataUnloaded(mapdata));
  }

  init() {
    on('we:map', (mapString: string) => {
      this.map = JSON.parse(mapString);

      setTimeout(() => {
        this.applyLoadedMap();

        sendSdkMessage('we:ready');
      }, 0);
    });

    sendSdkBackendMessage('we:accept');
  }

  update() {
    if (!this.map) {
      return;
    }

    const cam = limitPrecision(CameraManager.getCam(), 10000);
    const camString = JSON.stringify(cam);
    if (camString !== this.lastCamString) {
      this.lastCamString = camString;
      sendSdkBackendMessage('we:setCam', cam);
    }

    this.updateSelectedEntity();
  }

  destroy() {
    Citizen.invokeNative(joaat('DISABLE_EDITOR_RUNTIME'));

    this.map = null;
    this.pendingMapdatas = [];
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

  private applyLoadedMap() {
    CameraManager.setCam(this.map.meta.cam);

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

  private storeMapdataPatch(mapdata: number, entity: number, mat: Float32Array | WorldEditorEntityMatrix) {
    const matCopy = limitPrecision(Array.from(mat), 10000);
    if (matCopy.length !== 16) {
      return;
    }

    this.map.patches[mapdata] ??= {};
    this.map.patches[mapdata][entity] = {
      cam: CameraManager.getCam(),
      mat: matCopy as WorldEditorEntityMatrix,
    };

    applyMapdataPatch(mapdata, entity, mat);

    sendSdkBackendMessage('we:applyPatch', {
      mapDataHash: mapdata,
      entityHash: entity,
      patch: {
        cam: limitPrecision(CameraManager.getCam(), 10000),
        mat: matCopy,
      },
    });
  }

  private updateSelectedEntity() {
    const selectedEntity = SelectionController.getSelectedEntity();

    if (selectedEntity !== null) {
      const entityMatrix = makeEntityMatrix(selectedEntity);

      if (DrawGizmo(entityMatrix as any, selectedEntity.toString())) {
        applyEntityMatrix(selectedEntity, entityMatrix);

        const [success, mapdata, entity] = GetEntityMapdataOwner(selectedEntity);
        if (success) {
          this.storeMapdataPatch(mapdata, entity, entityMatrix);
        }
      }
    }
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
