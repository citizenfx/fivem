import { WORLD_EDITOR_MAP_NO_GROUP } from "backend/world-editor/world-editor-constants";
import { WEApi } from "backend/world-editor/world-editor-game-api";
import {
  WEApplyAdditionChangeRequest,
  WEApplyPatchChangeRequest,
  WECreatePatchRequest,
  WECam,
  WECreateAdditionGroupRequest,
  WECreateAdditionRequest,
  WEDeleteAdditionGroupRequest,
  WEDeleteAdditionRequest,
  WEDeletePatchRequest,
  WEEntityMatrixIndex,
  WEMap,
  WEMapAddition,
  WEMapAdditionGroup,
  WEMapPatch,
  WESetAdditionGroupNameRequest,
  WESetAdditionRequest,
  WEMapAdditionGroupDefinition,
  WESelectionType,
} from "backend/world-editor/world-editor-types";
import { makeAutoObservable } from "mobx";
import { worldEditorApi } from "shared/api.events";
import { applyRotation, applyScale } from "shared/math";
import { sendApiMessage } from "utils/api";
import { toNumber } from "utils/conversion";
import { omit } from "utils/omit";
import { pick } from "utils/pick";
import { fastRandomId } from "utils/random";
import { invokeWEApi } from "../we-api-utils";
import { WEEvents } from "./Events";
import { WEHistory } from "./history/WEHistory";
import { WEState } from "./WEState";

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

  handleCreatePatchRequest({ mapDataHash, entityHash, patch }: WECreatePatchRequest) {
    WEHistory.patchCreated(mapDataHash, entityHash, patch);

    if (!this.map.patches[mapDataHash]) {
      this.map.patches[mapDataHash] = {};
    }

    this.map.patches[mapDataHash][entityHash] = patch;
  }

  handleApplyPatchChangeRequest(request: WEApplyPatchChangeRequest) {
    const patch = this.getPatch(request.mapdata, request.entity);
    if (!patch) {
      return;
    }

    WEHistory.beginPatchChange(request.mapdata, request.entity, patch);

    if (request.mat) {
      patch.mat = request.mat;
    }

    if (request.label) {
      patch.label = request.label;
    }

    if (request.cam) {
      patch.cam = request.cam;
    }

    WEHistory.finishPatchChange(patch);
  }

  handleApplyAdditionChangeRequest(request: WEApplyAdditionChangeRequest) {
    const addition = this.map.additions[request.id];
    if (addition) {
      WEHistory.beginAdditionChange(request.id, addition);

      const newAddition = {
        ...addition,
        ...omit(request, 'id'),
      };

      WEHistory.finishAdditionChange(newAddition);

      this.map.additions[request.id] = newAddition;
      this.updateAdditionInGroups(request.id);
    }
  }

  readonly handleAdditionPlaced = ({ id, cam, mat }) => {
    const addition = this.map.additions[id];
    if (addition) {
      addition.cam = cam;
      addition.mat = mat;

      this.updateAdditionInGroups(id);

      WEHistory.additionCreated(id, addition);
    }
  };

  setPatch(mapdata: number, entity: number, patch: WEMapPatch) {
    if (!this.map.patches[mapdata]) {
      this.map.patches[mapdata] = {};
    }

    this.map.patches[mapdata][entity] = patch;

    const request = {
      mapDataHash: mapdata,
      entityHash: entity,
      patch,
    };

    invokeWEApi(WEApi.PatchCreate, request);
    sendApiMessage(worldEditorApi.createPatch, request);
  }

  getPatch(mapdata: number | string, entity: number | string): WEMapPatch | void {
    return this.map.patches[mapdata]?.[entity];
  }

  readonly setPatchPosition = this.patchChangeWrap(['mat'], (patch, x: number, y: number, z: number) => {
    patch.mat[WEEntityMatrixIndex.AX] = x;
    patch.mat[WEEntityMatrixIndex.AY] = y;
    patch.mat[WEEntityMatrixIndex.AZ] = z;
  });

  readonly setPatchRotation = this.patchChangeWrap(['mat'], (patch, x: number, y: number, z: number) => {
    applyRotation(patch.mat, [z, x, y]);
  });

  readonly setPatchScale = this.patchChangeWrap(['mat'], (patch, x: number, y: number, z: number) => {
    applyScale(patch.mat, [x, y, z]);
  });

  getGroupAdditions(grp: string): Record<string, WEMapAddition> {
    return this.groupedAdditions[grp] || {};
  }

  setAddition(id: string, addition: WEMapAddition) {
    this.additions[id] = addition;
    this.updateAdditionInGroups(id);

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
    this.updateAdditionInGroups(id);

    const request: WECreateAdditionRequest = {
      id,
      addition,
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

  deletePatch(mapdata: string | number, entity: string | number, withHistoryEntry = true) {
    const patch = this.getPatch(mapdata, entity);
    if (!patch) {
      return;
    }

    if (withHistoryEntry) {
      WEHistory.patchDeleted(toNumber(mapdata), toNumber(entity), patch);
    }

    delete this.map.patches[mapdata][entity];

    if (Object.keys(this.map.patches[mapdata]).length === 0) {
      delete this.map.patches[mapdata];
    }

    const request: WEDeletePatchRequest = {
      mapDataHash: toNumber(mapdata),
      entityHash: toNumber(entity),
    };

    WEEvents.emitPatchDeleted(request.mapDataHash, request.entityHash, patch);

    invokeWEApi(WEApi.PatchDelete, request);
    sendApiMessage(worldEditorApi.deletePatch, request);
  }

  deleteAddition(id: string, withHistoryEntry = true) {
    const addition = this.map.additions[id];
    if (addition) {
      if (addition.grp !== WORLD_EDITOR_MAP_NO_GROUP && this.groupedAdditions[addition.grp]) {
        delete this.groupedAdditions[addition.grp][id];
      } else {
        delete this.ungroupedAdditions[id];
      }

      delete this.map.additions[id];

      if (withHistoryEntry) {
        WEHistory.additionDeleted(id, addition);
      }

      WEEvents.emitAdditionDeleted(id, addition);

      invokeWEApi(WEApi.AdditionDelete, id);
      sendApiMessage(worldEditorApi.deleteAddition, {
        id
      } as WEDeleteAdditionRequest);
    }
  }

  setAdditionGroup(id: string, grp: WEMapAdditionGroup, withHistoryEntry = true) {
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

      if (withHistoryEntry) {
        WEHistory.additionGroupChanged(id, addition.grp, grp);
      }

      addition.grp = grp;
      this.updateAdditionInGroups(id);

      this.broadcastAdditionChange(id, { grp });
    }
  }

  setAdditionsGroup(grp: string, definition: WEMapAdditionGroupDefinition) {
    this.map.additionGroups[grp] = definition;
    this.groupedAdditions[grp] = {};

    sendApiMessage(worldEditorApi.createAdditionGroup, {
      grp,
      label: definition.label,
    } as WECreateAdditionGroupRequest);
  }

  createAdditionGroup(label: string): string {
    const grp = fastRandomId();

    this.setAdditionsGroup(grp, { label });

    return grp;
  }

  restoreAdditionsGroup(grp: string, definition: WEMapAdditionGroupDefinition, additions: Record<string, WEMapAddition>) {
    this.setAdditionsGroup(grp, definition);

    if (Object.keys(additions).length > 0) {
      for (const [id, addition] of Object.entries(additions)) {
        this.setAddition(id, addition);
      }
    }
  }

  deleteAdditionGroup(grp: string, deleteAdditions: boolean, withHistoryEntry = true) {
    const ids = Object.keys(this.groupedAdditions[grp] || {});

    if (withHistoryEntry) {
      if (deleteAdditions) {
        WEHistory.additionsGroupDeleted(
          grp,
          this.map.additionGroups[grp],
          this.groupedAdditions[grp],
        );
      } else {
        WEHistory.additionsGroupDeleted(
          grp,
          this.map.additionGroups[grp],
        );
      }
    }

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

  setAdditionsGroupDefinition(grp: string, definition: WEMapAdditionGroupDefinition) {
    this.map.additionGroups[grp] = definition;

    sendApiMessage(worldEditorApi.setAdditionGroupLabel, {
      grp,
      label: definition.label,
    } as WESetAdditionGroupNameRequest);
  }

  setAdditionGroupLabel(grp: string, label: string) {
    if (!label) {
      return;
    }

    const definition = this.map.additionGroups[grp];
    const newDefinition = {
      ...definition,
      label,
    };

    this.map.additionGroups[grp] = newDefinition;

    WEHistory.additionsGroupChanged(grp, definition, newDefinition);

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

  private patchChangeWrap<Args extends any[], ChangeKeys extends keyof WEMapPatch>(
    keys: ChangeKeys[],
    fn: (patch: WEMapPatch, ...args: Args) => void
  ): ((mapdata: number | string, entity: number | string, ...args: Args) => void) {
    return (mapdata, entity, ...args: Args) => {
      let patch = this.getPatch(mapdata, entity);
      if (!patch) {
        const result = this.createPatchFromSelection();
        if (!result) {
          return;
        }

        fn(result.patch, ...args);

        WEHistory.patchCreated(result.mapdata, result.entity, result.patch);

        this.setPatch(result.mapdata, result.entity, result.patch);

        return;
      }

      const mapdataNumber = toNumber(mapdata);
      const entityNumber = toNumber(entity);

      WEHistory.beginPatchChange(mapdataNumber, entityNumber, patch);

      fn(patch, ...args);

      WEHistory.finishPatchChange(patch);

      this.broadcastPatchChange(mapdataNumber, entityNumber, pick(patch, ...keys));
    };
  }

  private createPatchFromSelection(): { mapdata: number, entity: number, patch: WEMapPatch } | null {
    if (WEState.selection.type !== WESelectionType.PATCH) {
      return null;
    }
    if (!WEState.selection.mat || !WEState.selection.cam) {
      return null;
    }

    const patch: WEMapPatch = {
      mat: WEState.selection.mat,
      cam: WEState.selection.cam,
      label: WEState.selection.label,
    };

    return {
      patch,
      mapdata: WEState.selection.mapdata,
      entity: WEState.selection.entity,
    };
  }

  private broadcastAdditionChange(id: string, change: Partial<WEMapAddition>) {
    const request: WEApplyAdditionChangeRequest = {
      id,
      ...change,
    };

    invokeWEApi(WEApi.AdditionApplyChange, request);
    sendApiMessage(worldEditorApi.applyAdditionChange, request);
  }

  private broadcastPatchChange(mapdata: number, entity: number, change: Partial<WEMapPatch>) {
    const request: WEApplyPatchChangeRequest = {
      mapdata,
      entity,
      ...change,
    };

    invokeWEApi(WEApi.PatchApplyChange, request);
    sendApiMessage(worldEditorApi.applyPatchChange, request);
  }

  private updateAdditionInGroups(id: string) {
    const addition = this.map.additions[id];
    if (addition) {
      if (addition.grp === WORLD_EDITOR_MAP_NO_GROUP) {
        this.ungroupedAdditions[id] = addition;
      } else {
        if (!this.groupedAdditions[addition.grp]) {
          this.groupedAdditions[addition.grp] = {};
        }

        this.groupedAdditions[addition.grp][id] = addition;
      }
    }
  }
}
