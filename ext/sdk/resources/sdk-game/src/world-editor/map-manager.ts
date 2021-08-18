import { joaat } from "../shared";
import { CameraManager } from "./camera-manager";
import {
  WEApplyAdditionChangeRequest,
  WECreateAdditionRequest,
  WEEntityMatrix,
  WEMap,
  WEMapAddition,
  WEMapPatch,
  WESelectionType,
  WESelection,
} from "@sdk-root/backend/world-editor/world-editor-types";
import { WEApi } from '@sdk-root/backend/world-editor/world-editor-game-api';
import { applyAdditionMatrix, applyEntityMatrix, makeEntityMatrix, Vec3 } from "./math";
import { ObjectManager } from "./object-manager";
import { SelectionController } from "./selection-controller";
import { SettingsManager } from "./settings-manager";
import { drawDebugText, invokeWEApi, invokeWEApiBackend, invokeWEApiBroadcast, onWEApi } from "./utils";
import { PatchManager, UpdateOrCreateResult } from "./patch-manager";

export const MapManager = new class MapManager {
  private enabled = true;

  private map: WEMap | null = null;
  private lastCamString: string = '';

  private objects: ObjectManager | null = null;
  private patches: PatchManager | null = null;

  public selection: WESelection = { type: WESelectionType.NONE };
  private selectionHandle: number | null = null;

  enable() {
    this.enabled = true;
  }

  disable() {
    this.enabled = false;
  }

  preinit() {
    EnableEditorRuntime();

    SelectionController.onSelectionChanged(this.handleSelectionChanged);

    onWEApi(WEApi.Selection, (selection) => {
      this.selection = selection;
      this.selectionHandle = null;

      SelectionController.setSelectedEntity(null);
    });

    onWEApi(WEApi.AdditionCreate, this.handleSpawnObject);

    onWEApi(WEApi.AdditionSet, ({ id, addition }) => {
      this.map.additions[id] = addition;
      this.objects.set(id, addition);
    });

    onWEApi(WEApi.AdditionApplyChange, this.handleApplyAdditionChange);

    onWEApi(WEApi.AdditionSetOnGround, (additionId) => {
      const addition = this.map.additions[additionId];
      if (!addition) {
        return;
      }

      const handle = this.objects.getObjectHandle(additionId);
      if (handle !== null) {
        PlaceObjectOnGroundProperly(handle);

        this.updateAddition(additionId, addition, makeEntityMatrix(handle));
      }
    });

    onWEApi(WEApi.PatchCreate, ({ mapDataHash, entityHash, patch }) => {
      this.patches.set(mapDataHash, entityHash, patch);
    });

    onWEApi(WEApi.PatchApplyChange, (request) => {
      if (request.mat) {
        this.patches.updateMat(request.mapdata, request.entity, request.mat);
      }
    });

    onWEApi(WEApi.PatchDelete, ({ mapDataHash, entityHash }) => {
      this.patches.delete(mapDataHash, entityHash);
    });

    onWEApi(WEApi.AdditionDelete, this.handleDeleteAddition);
    onWEApi(WEApi.AdditionDeleteBatch, this.handleDeleteAdditions);

    onWEApi(WEApi.Map, (map) => {
      this.map = map;

      setTimeout(() => {
        this.applyLoadedMap();

        invokeWEApi(WEApi.Ready, undefined);
      }, 0);
    });

    onWEApi(WEApi.SetCam, (cam) => CameraManager.setCam(cam));

    onWEApi(WEApi.FocusInView, (request) => {
      CameraManager.setCam(request.cam);
      CameraManager.setLookAt(...request.lookAt);
    });
  }

  init() {
    invokeWEApiBackend(WEApi.Accept, undefined);
  }

  update() {
    if (!this.map) {
      return;
    }

    if (this.objects) {
      this.objects.update();
    }

    if (this.enabled) {
      const cam = CameraManager.getCamLimitedPrecision();
      const camString = JSON.stringify(cam);
      if (camString !== this.lastCamString) {
        this.lastCamString = camString;

        invokeWEApiBackend(WEApi.SetCam, cam);
      }

      this.updateSelectedEntity();
    }
  }

  private handleSelectionChanged = ([current, _previous]) => {
    this.selectionHandle = current;

    if (!current) {
      this.selection = {
        type: WESelectionType.NONE,
      };

      return invokeWEApi(WEApi.Selection, this.selection);
    }

    const additionId = this.objects.getObjectId(current);

    if (additionId) {
      this.selection = {
        type: WESelectionType.ADDITION,
        id: additionId,
      };

      return invokeWEApi(WEApi.Selection, this.selection);
    }

    const [success, mapdata, entity] = GetEntityMapdataOwner(current);
    if (success) {
      this.selection = {
        type: WESelectionType.PATCH,
        mapdata,
        entity,
        label: this.getEntityLabel(current),
      };

      return invokeWEApi(WEApi.Selection, this.selection);
    }
  };

  private updateSelectedEntity() {
    // Happens when dummy<->instantiated transition happens for mapdata entities
    if (this.selectionHandle !== null && !DoesEntityExist(this.selectionHandle)) {
      console.log('SELECTED ENTITY DOES NOT EXIST ANYMORE', this.selectionHandle);
      this.selectionHandle = null;
      SelectionController.setSelectedEntity(this.selectionHandle);
    }

    // No selection handle but active selection, try acquiring handle
    if (this.selectionHandle === null && this.selection.type !== WESelectionType.NONE) {
      switch (this.selection.type) {
        case WESelectionType.ADDITION: {
          this.selectionHandle = this.objects.getObjectHandle(this.selection.id);

          break;
        }
        case WESelectionType.PATCH: {
          this.selectionHandle = this.patches.getHandle(this.selection.mapdata, this.selection.entity);

          if (this.selectionHandle !== null) {
            console.log('ACQUIRED NEW HANDLE FOR SELECTED PATCH', this.selectionHandle);
          }

          break;
        }
      }

      SelectionController.setSelectedEntity(this.selectionHandle);
    }

    if (this.selectionHandle === null) {
      return;
    }

    if (SettingsManager.settings.showSelectionBoundingBox) {
      this.drawBoundingBox(this.selectionHandle);
    }

    const entityMatrix = makeEntityMatrix(this.selectionHandle);

    if (DrawGizmo(entityMatrix as any, this.selectionHandle.toString())) {
      switch (this.selection.type) {
        case WESelectionType.ADDITION: {
          const addition = this.map.additions[this.selection.id];

          applyAdditionMatrix(this.selectionHandle, Array.from(entityMatrix));
          this.updateAddition(this.selection.id, addition, entityMatrix);

          break;
        }
        case WESelectionType.PATCH: {
          applyEntityMatrix(this.selectionHandle, entityMatrix);
          this.updatePatch(this.selectionHandle, entityMatrix);

          break;
        }
      }
    }
  }

  destroy() {
    Citizen.invokeNative(joaat('DISABLE_EDITOR_RUNTIME'));

    if (this.objects) {
      this.objects.destroy();
      this.objects = null;
    }

    this.map = null;
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

  private readonly handleSpawnObject = (request: WECreateAdditionRequest) => {
    const pos = getObjectPosition();

    request.addition.cam = CameraManager.getCamLimitedPrecision();
    request.addition.mat = prepareEntityMatrix([
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      pos.x, pos.y, pos.z, 1,
    ]);

    invokeWEApiBroadcast(WEApi.AdditionPlaced, {
      id: request.id,
      cam: request.addition.cam,
      mat: request.addition.mat,
    });

    this.map.additions[request.id] = request.addition;
    this.objects.set(request.id, request.addition);
  };

  handleDeletePatch(mapdataHash: number, entityHash: number) {
    this.patches.delete(mapdataHash, entityHash);
  }

  private readonly handleDeleteAddition = (objectId: string) => {
    this.handleDeleteAdditions([objectId]);
  };

  private readonly handleDeleteAdditions = (ids: string[]) => {
    if (this.map) {
      for (const id of ids) {
        delete this.map.additions[id];
        this.objects.delete(id);
      }
    }
  };

  private readonly handleApplyAdditionChange = (request: WEApplyAdditionChangeRequest) => {
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
  };

  private applyLoadedMap() {
    CameraManager.setCam(this.map.meta.cam);

    this.objects = new ObjectManager(this.map.additions);
    this.objects.onObjectCreated((additionId: string, handle: number) => {
      if (this.selection.type === WESelectionType.ADDITION && this.selection.id === additionId) {
        this.selectionHandle = handle;
        SelectionController.setSelectedEntity(handle);
      }
    });

    this.patches = new PatchManager(this.map.patches);
  }

  private updatePatch(entityGuid: number, rawmat: Float32Array | WEEntityMatrix) {
    const mat = prepareEntityMatrix(rawmat);

    const [result, mapdata, entity] = this.patches.updateOrCreate(entityGuid, mat);

    switch (result) {
      case UpdateOrCreateResult.CREATE: {
        const patch: WEMapPatch = {
          label: this.getEntityLabel(entityGuid),
          cam: CameraManager.getCamLimitedPrecision(),
          mat,
        };

        this.map.patches[mapdata] ??= {};
        this.map.patches[mapdata][entity] = patch;

        invokeWEApiBroadcast(WEApi.PatchCreate, {
          mapDataHash: mapdata,
          entityHash: entity,
          patch,
        });
      }
      case UpdateOrCreateResult.UPDATE: {
        const patch = this.map.patches[mapdata][entity];

        patch.cam = CameraManager.getCamLimitedPrecision();
        patch.mat = mat;

        invokeWEApiBroadcast(WEApi.PatchApplyChange, {
          mapdata,
          entity,
          mat: patch.mat,
          cam: patch.cam,
        });
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

    invokeWEApiBroadcast(WEApi.AdditionApplyChange, change);
  }

  private getEntityLabel(entity: number): string {
    return GetEntityArchetypeName(entity) || GetEntityModel(entity).toString(16).toUpperCase();
  }
};

function getObjectPosition() {
  const cp = CameraManager.getPosition();
  const fw = CameraManager.getForwardVector().copy().mult(100);

  const rh = StartExpensiveSynchronousShapeTestLosProbe(
    cp.x, cp.y, cp.z,
    cp.x + fw.x, cp.y + fw.y, cp.z + fw.z,
    1 | 16 | 256,
    0,
    4,
  );

  const [, hit, endCoords] = GetShapeTestResult(rh);

  if (hit) {
    return new Vec3(endCoords[0], endCoords[1], endCoords[2]);
  }

  return CameraManager.getPosition().copy().add(CameraManager.getForwardVector().copy().mult(3));
}

function prepareEntityMatrix(mat: Float32Array | WEEntityMatrix): WEEntityMatrix {
  return Array.from(mat) as any;
}

function drawLine(f: [number, number, number] | number[], t: [number, number, number] | number[], c: [number, number, number, number] | number[]) {
  DrawLine(f[0], f[1], f[2], t[0], t[1], t[2], c[0], c[1], c[2], c[3]);
}
