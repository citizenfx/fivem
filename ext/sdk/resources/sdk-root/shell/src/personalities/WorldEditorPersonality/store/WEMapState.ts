import { WORLD_EDITOR_MAP_NO_GROUP } from "backend/world-editor/world-editor-constants";
import { WEApi } from "backend/world-editor/world-editor-game-api";
import { WEApplyAdditionChangeRequest, WEApplyPatchChangeRequest, WECreatePatchRequest, WECam, WECreateAdditionGroupRequest, WECreateAdditionRequest, WEDeleteAdditionGroupRequest, WEDeleteAdditionRequest, WEDeletePatchRequest, WEEntityMatrixIndex, WEMap, WEMapAddition, WEMapAdditionGroup, WEMapPatch, WESetAdditionGroupNameRequest, WESetAdditionRequest } from "backend/world-editor/world-editor-types";
import { makeAutoObservable } from "mobx";
import { worldEditorApi } from "shared/api.events";
import { applyRotation, applyScale } from "shared/math";
import { sendApiMessage } from "utils/api";
import { omit } from "utils/omit";
import { pick } from "utils/pick";
import { fastRandomId } from "utils/random";
import { number } from "yargs";
import { invokeWEApi } from "../we-api-utils";
import { WEEvents } from "./Events";
import { WEHistory } from "./history/WEHistory";

export class WEMapState {
  private ungroupedAdditions: Record<string, WEMapAddition> = {};
  private groupedAdditions: Record<number, Record<string, WEMapAddition>> = {};

  constructor(private map: WEMap) {
    makeAutoObservable(this);

    for (const [additionId, addition] of Object.entries(map.additions)) {
      if (addition.grp === WORLD_EDITOR_MAP_NO_GROUP) {
        this.ungroupedAdditions[additionId] = addition;
      } else {
        if (!this.groupedAdditions[addition.grp]) {
          this.groupedAdditions[addition.grp] = {};
        }

        this.groupedAdditions[addition.grp][additionId] = addition;
      }
    }
  }

  get cam(): WECam {
    return this.map.meta.cam;
  }

  get additions(): Record<string, WEMapAddition> {
    return this.map.additions;
  }

  get additionGroups() {
    return this.map.additionGroups;
  }

  get additionsByGroups(): Record<number, Record<string, WEMapAddition>> {
    return this.groupedAdditions;
  }

  get additionsUngrouped(): Record<string, WEMapAddition> {
    return this.ungroupedAdditions;
  }

  get patches(): Record<string, Record<string, WEMapPatch>> {
    return this.map.patches;
  }

  handleCreatePatchRequest({ entityHash, mapDataHash, patch }: WECreatePatchRequest) {
    if (!this.map.patches[mapDataHash]) {
      this.map.patches[mapDataHash] = {};
    }

    this.map.patches[mapDataHash][entityHash] = patch;
  }

  handleApplyPatchChangeRequest(request: WEApplyPatchChangeRequest) {
    const patch = this.map.patches[request.mapdata]?.[request.entity];
    if (!patch) {
      return;
    }

    if (request.mat) {
      patch.mat = request.mat;
    }

    if (request.label) {
      patch.label = request.label;
    }

    if (request.cam) {
      patch.cam = request.cam;
    }
  }

  handleApplyAdditionChangeRequest(request: WEApplyAdditionChangeRequest) {
    const addition = this.map.additions[request.id];
    if (addition) {
      const setAdditionRequest: WESetAdditionRequest = {
        id: request.id,
        addition: {
          ...addition,
          ...omit(request, 'id'),
        },
      };

      this.handleSetAdditionRequest(setAdditionRequest);
    }
  }

  handleSetAdditionRequest({ id, addition }: WESetAdditionRequest) {
    this.map.additions[id] = addition;

    if (addition.grp !== -1) {
      if (!this.groupedAdditions[addition.grp]) {
        this.groupedAdditions[addition.grp] = {};
      }

      this.groupedAdditions[addition.grp][id] = addition;
    } else {
      this.ungroupedAdditions[id] = addition;
    }
  }

  getPatch(mapdata: number, entity: number): WEMapPatch | void {
    return this.map.patches[mapdata]?.[entity];
  }

  setPatchPosition(mapdata: number, entity: number, x: number, y: number, z: number) {
    const patch = this.map.patches[mapdata]?.[entity];
    if (!patch) {
      return;
    }

    patch.mat[WEEntityMatrixIndex.AX] = x;
    patch.mat[WEEntityMatrixIndex.AY] = y;
    patch.mat[WEEntityMatrixIndex.AZ] = z;

    const request: WEApplyPatchChangeRequest = {
      entity,
      mapdata,

      mat: patch.mat,
    };

    invokeWEApi(WEApi.PatchApplyChange, request);
    sendApiMessage(worldEditorApi.applyPatchChange, request);
  }

  setPatchRotation(mapdata: number, entity: number, x: number, y: number, z: number) {
    const patch = this.map.patches[mapdata]?.[entity];
    if (!patch) {
      return;
    }

    applyRotation(patch.mat, [z, x, y]);

    const request: WEApplyPatchChangeRequest = {
      entity,
      mapdata,

      mat: patch.mat,
    };

    invokeWEApi(WEApi.PatchApplyChange, request);
    sendApiMessage(worldEditorApi.applyPatchChange, request);
  }

  setPatchScale(mapdata: number, entity: number, x: number, y: number, z: number) {
    const patch = this.map.patches[mapdata]?.[entity];
    if (!patch) {
      return;
    }

    applyScale(patch.mat, [x, y, z]);

    const request: WEApplyPatchChangeRequest = {
      entity,
      mapdata,

      mat: patch.mat,
    };

    invokeWEApi(WEApi.PatchApplyChange, request);
    sendApiMessage(worldEditorApi.applyPatchChange, request);
  }

  getGroupAdditions(grp: string): Record<string, WEMapAddition> {
    return this.groupedAdditions[grp] || {};
  }

  setAddition(id: string, addition: WEMapAddition) {
    this.additions[id] = addition;

    if (addition.grp === WORLD_EDITOR_MAP_NO_GROUP) {
      this.ungroupedAdditions[id] = addition;
    } else {
      if (!this.groupedAdditions[addition.grp]) {
        this.groupedAdditions[addition.grp] = {};
      }

      this.groupedAdditions[addition.grp][id] = addition;
    }

    const request: WESetAdditionRequest = {
      id,
      addition,
    };

    invokeWEApi(WEApi.AdditionSet, request);
    sendApiMessage(worldEditorApi.setAddition, request);
  }

  createAddition(mdl: string, grp: WEMapAdditionGroup) {
    const id = fastRandomId();

    const addition: WEMapAddition = {
      mdl,
      grp,
      cam: [0, 0, 100, 0, 0, -45],
      label: mdl,
      mat: [
        1, 0, 0, 1,
        0, 1, 0, 1,
        0, 0, 1, 1,
        0, 0, 0, 1,
      ],
    };

    this.map.additions[id] = addition;

    if (grp === WORLD_EDITOR_MAP_NO_GROUP) {
      this.ungroupedAdditions[id] = addition;
    } else {
      if (!this.groupedAdditions[grp]) {
        this.groupedAdditions[grp] = {};
      }

      this.groupedAdditions[grp][id] = addition;
    }

    const request: WECreateAdditionRequest = {
      id,
      addition,
      needsPlacement: true,
    };

    invokeWEApi(WEApi.AdditionCreate, request);
    sendApiMessage(worldEditorApi.createAddition, request);
  }

  readonly setAdditionPosition = this.additionChangeWrap(['mat'], (addition, x: number, y: number, z: number) => {
    addition.mat[WEEntityMatrixIndex.AX] = x;
    addition.mat[WEEntityMatrixIndex.AY] = y;
    addition.mat[WEEntityMatrixIndex.AZ] = z;
  });

  readonly setAdditionRotation = this.additionChangeWrap(['mat'], (addition, x: number, y: number, z: number) => {
    applyRotation(addition.mat, [z, x, y]);
  });

  readonly setAdditionScale = this.additionChangeWrap(['mat'], (addition, x: number, y: number, z: number) => {
    applyScale(addition.mat, [x, y, z]);
  });

  readonly setAdditionLabel = this.additionChangeWrap(['label'], (addition, label: string) => {
    addition.label = label;
  });

  setAdditionOnGround(additionId: string) {
    invokeWEApi(WEApi.AdditionSetOnGround, additionId);
  }

  deletePatch(mapdata: string | number, entity: string | number) {
    if (this.map.patches[mapdata]) {
      delete this.map.patches[mapdata][entity];

      if (Object.keys(this.map.patches[mapdata]).length === 0) {
        delete this.map.patches[mapdata];
      }
    }

    const request: WEDeletePatchRequest = {
      mapDataHash: typeof mapdata === 'string'
        ? parseInt(mapdata, 10)
        : mapdata,
      entityHash: typeof entity === 'string'
        ? parseInt(entity, 10)
        : entity,
    };

    invokeWEApi(WEApi.PatchDelete, request);
    sendApiMessage(worldEditorApi.deletePatch, request);
  }

  deleteAddition(id: string) {
    const addition = this.map.additions[id];
    if (addition) {
      if (addition.grp !== WORLD_EDITOR_MAP_NO_GROUP && this.groupedAdditions[addition.grp]) {
        delete this.groupedAdditions[addition.grp][id];
      } else {
        delete this.ungroupedAdditions[id];
      }

      delete this.map.additions[id];

      WEHistory.additionDeleted(id, addition);
      WEEvents.emitAdditionDeleted(id, addition);

      invokeWEApi(WEApi.AdditionDelete, id);
      sendApiMessage(worldEditorApi.deleteAddition, {
        id
      } as WEDeleteAdditionRequest);
    }
  }

  setAdditionGroup(id: string, grp: WEMapAdditionGroup) {
    const addition = this.map.additions[id];
    if (addition) {
      if (addition.grp === grp) {
        return;
      }

      if (addition.grp === WORLD_EDITOR_MAP_NO_GROUP) {
        delete this.ungroupedAdditions[id];
      } else {
        delete this.groupedAdditions[addition.grp][id];
      }

      addition.grp = grp;

      if (grp === WORLD_EDITOR_MAP_NO_GROUP) {
        this.ungroupedAdditions[id] = addition;
      } else {
        if (!this.groupedAdditions[grp]) {
          this.groupedAdditions[grp] = {};
        }

        this.groupedAdditions[grp][id] = addition;
      }

      this.broadcastAdditionChange(id, { grp });
    }
  }

  createAdditionGroup(label: string): string {
    const grp = fastRandomId();
    this.map.additionGroups[grp] = {
      label,
    };
    this.groupedAdditions[grp] = {};

    sendApiMessage(worldEditorApi.createAdditionGroup, {
      grp,
      label,
    } as WECreateAdditionGroupRequest);

    return grp;
  }

  deleteAdditionGroup(grp: string, deleteAdditions: boolean) {
    const ids = Object.keys(this.groupedAdditions[grp] || {});

    delete this.map.additionGroups[grp];
    delete this.groupedAdditions[grp];

    if (deleteAdditions) {
      for (const id of ids) {
        delete this.map.additions[id];
      }

      invokeWEApi(WEApi.AdditionDeleteBatch, ids);
    } else {
      for (const id of ids) {
        const addition = this.map.additions[id];
        addition.grp = WORLD_EDITOR_MAP_NO_GROUP;

        this.ungroupedAdditions[id] = addition;
      }
    }

    sendApiMessage(worldEditorApi.deleteAdditionGroup, {
      grp,
      deleteAdditions,
    } as WEDeleteAdditionGroupRequest);
  }

  setAdditionGroupLabel(grp: string, label: string) {
    if (!label) {
      return;
    }

    this.map.additionGroups[grp].label = label;

    sendApiMessage(worldEditorApi.setAdditionGroupLabel, {
      grp,
      label,
    } as WESetAdditionGroupNameRequest);
  }

  private additionChangeWrap<Args extends any[], ChangeKeys extends keyof WEMapAddition>(
    keys: ChangeKeys[],
    fn: (addition: WEMapAddition, ...args: Args) => void
  ): ((id: string, ...args: Args) => void) {
    return (id: string, ...args: Args) => {
      const addition = this.additions[id];
      if (!addition) {
        return;
      }

      WEHistory.beginAdditionChange(id, addition);

      fn(addition, ...args);

      WEHistory.finishAdditionChange(addition);

      this.broadcastAdditionChange(id, pick(addition, ...keys));
    };
  }

  private applyAdditionChange(id: string, change: Partial<WEMapAddition>) {
    const origin = this.map.additions[id];
    const grouped = this.groupedAdditions[origin.grp]?.[id];

    if (origin) {
      const newAddition = {
        ...origin,
        ...change,
      };

      this.map.additions[id] = newAddition;

      if (grouped) {
        this.groupedAdditions[newAddition.grp][id] = newAddition;
      } else {
        this.ungroupedAdditions[id] = newAddition;
      }

      this.broadcastAdditionChange(id, change);
    }
  }

  private broadcastAdditionChange(id: string, change: Partial<WEMapAddition>) {
    const request: WEApplyAdditionChangeRequest = {
      id,
      ...change,
    };

    invokeWEApi(WEApi.AdditionApplyChange, request);
    sendApiMessage(worldEditorApi.applyAdditionChange, request);
  }
}
