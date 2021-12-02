import { CameraManager } from "./camera-manager";
import {
  WEApplyAdditionChangeRequest,
  WECreateAdditionRequest,
  WEEntityMatrix,
  WEMap,
  WEMapPatch,
  WESelectionType,
  WESelection,
} from "@sdk-root/backend/world-editor/world-editor-types";
import { WEApi } from '@sdk-root/backend/world-editor/world-editor-game-api';
import { makeEntityMatrix } from "./math";
import { SelectionController } from "./selection-controller";
import { SettingsManager } from "./settings-manager";
import { invokeWEApi, invokeWEApiBackend, invokeWEApiBroadcast, onWEApi } from "./utils";
import { PatchManager, UpdateOrCreateResult } from "./patch-manager";
import { AdditionsManager } from "./additions-manager";

type NativeVector3 = [number, number, number];

export const MapManager = new class MapManager {
  private enabled = true;

  private map: WEMap | null = null;
  private lastCamString: string = '';

  private patches: PatchManager | null = null;

  private additions: AdditionsManager | null = null;

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

    onWEApi(WEApi.AdditionCreate, (request: WECreateAdditionRequest) => {
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

      this.additions.create(request.id, request.addition);
    });

    onWEApi(WEApi.AdditionSet, ({ id, addition }) => {
      if (!this.additions) {
        return;
      }

      if (this.additions.has(id)) {
        this.additions.updateAdditionFromShell(id, addition.mat);
      } else {
        this.additions.create(id, addition);
      }
    });

    onWEApi(WEApi.AdditionApplyChange, ({ id, mat }: WEApplyAdditionChangeRequest) => {
      /**
       * We only care about matrix updates here
       */
      if (!mat) {
        return;
      }

      this.additions?.updateAdditionFromShell(id, mat);
    });

    onWEApi(WEApi.AdditionSetOnGround, (additionId) => {
      this.additions?.setAdditionOnGround(additionId)
    });

    onWEApi(WEApi.AdditionDelete, (additionId: string) => {
      this.additions?.delete(additionId);
    });
    onWEApi(WEApi.AdditionDeleteBatch, (additionIds: string[]) => {
      if (!this.additions) {
        return;
      }

      for (const additionId of additionIds) {
        this.additions.delete(additionId);
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

    this.additions.update();

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

    const additionId = this.additions.getAdditionIdFromHandle(current);
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

      if (!this.patches[mapdata]?.[entity]) {
        this.selection.mat = prepareEntityMatrix(makeEntityMatrix(current));
        this.selection.cam = CameraManager.getCamLimitedPrecision();
      }

      return invokeWEApi(WEApi.Selection, this.selection);
    }
  };

  private updateSelectedEntity() {
    // Happens when dummy<->instantiated transition happens for mapdata entities
    if (this.selectionHandle !== null && !DoesEntityExist(this.selectionHandle)) {
      this.selectionHandle = null;
      SelectionController.setSelectedEntity(this.selectionHandle);
    }

    // No selection handle but something's in selection, try acquiring handle
    if (this.selectionHandle === null && this.selection.type !== WESelectionType.NONE) {
      switch (this.selection.type) {
        case WESelectionType.ADDITION: {
          this.selectionHandle = this.additions.getHandleFromAddition(this.selection.id);
          if (this.selectionHandle === undefined) {
            this.selectionHandle = null;
          }

          break;
        }
        case WESelectionType.PATCH: {
          this.selectionHandle = this.patches.getHandle(this.selection.mapdata, this.selection.entity);

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
          this.additions.updateAdditionFromGizmo(this.selection.id, entityMatrix);
          break;
        }
        case WESelectionType.PATCH: {
          this.updatePatch(this.selectionHandle, entityMatrix);

          break;
        }
      }
    }
  }

  destroy() {
    DisableEditorRuntime();

    this.additions?.dispose();
    this.additions = null;

    this.map = null;
  }

  private drawBoundingBox(entity: number) {
    const [min, max] = GetModelDimensions(GetEntityModel(entity)) as [NativeVector3, NativeVector3];

    const AF = translateEntityCoords(entity, min);
    // drawTextAt('AF', AF);
    const BF = translateEntityCoords(entity, [min[0], min[1], max[2]]);
    // drawTextAt('BF', BF);
    const CF = translateEntityCoords(entity, [min[0], max[1], max[2]]);
    // drawTextAt('CF', CF);
    const DF = translateEntityCoords(entity, [min[0], max[1], min[2]]);
    // drawTextAt('DF', DF);

    const AB = translateEntityCoords(entity, [max[0], min[1], min[2]]);
    // drawTextAt('AB', AB);
    const BB = translateEntityCoords(entity, [max[0], min[1], max[2]]);
    // drawTextAt('BB', BB);
    const CB = translateEntityCoords(entity, max);
    // drawTextAt('CB', CB);
    const DB = translateEntityCoords(entity, [max[0], max[1], min[2]]);
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

  handleDeletePatch(mapdataHash: number, entityHash: number) {
    this.patches.delete(mapdataHash, entityHash);
  }

  private applyLoadedMap() {
    CameraManager.setCam(this.map.meta.cam);

    this.patches = new PatchManager(this.map.patches);
    this.additions = new AdditionsManager(this.map.additions, (additionId, handle) => {
      if (this.selection.type === WESelectionType.ADDITION && this.selection.id === additionId) {
        this.selectionHandle = handle;
        SelectionController.setSelectedEntity(handle);
      }
    });
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

  private getEntityLabel(entity: number): string {
    return GetEntityArchetypeName(entity) || GetEntityModel(entity).toString(16).toUpperCase();
  }
};

function getObjectPosition() {
  // const cp = CameraManager.getPosition();
  // const fw = CameraManager.getForwardVector().copy().mult(100);

  // const rh = StartExpensiveSynchronousShapeTestLosProbe(
  //   cp.x, cp.y, cp.z,
  //   cp.x + fw.x, cp.y + fw.y, cp.z + fw.z,
  //   1 | 16 | 256,
  //   0,
  //   4,
  // );

  // const [, hit, endCoords] = GetShapeTestResult(rh);

  // if (hit) {
  //   return new Vec3(endCoords[0], endCoords[1], endCoords[2]);
  // }

  return CameraManager.getPosition().copy().add(CameraManager.getForwardVector().copy().mult(5));
}

function prepareEntityMatrix(mat: Float32Array | WEEntityMatrix): WEEntityMatrix {
  return Array.from(mat) as any;
}

function drawLine(f: NativeVector3 | number[], t: NativeVector3 | number[], c: [number, number, number, number] | number[]) {
  DrawLine(f[0], f[1], f[2], t[0], t[1], t[2], c[0], c[1], c[2], c[3]);
}

function translateEntityCoords(entity: number, [x, y, z]: NativeVector3): NativeVector3 {
  return GetOffsetFromEntityInWorldCoords(entity, x, y, z) as NativeVector3;
}
