import { WORLD_EDITOR_MAP_NO_GROUP } from "backend/world-editor/world-editor-constants";
import { WEApplyAdditionChangeRequest, WEApplyPatchRequest, WECam, WECreateAdditionGroupRequest, WEDeleteAdditionGroupRequest, WEDeleteAdditionRequest, WEMap, WEMapAddition, WEMapAdditionGroup, WEMapPatch, WESetAdditionGroupNameRequest, WESetAdditionRequest } from "backend/world-editor/world-editor-types";
import { makeAutoObservable } from "mobx";
import { worldEditorApi } from "shared/api.events";
import { sendApiMessage } from "utils/api";
import { omit } from "utils/omit";
import { fastRandomId } from "utils/random";

export class WorldEditorMapState {
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

  handleApplyPatchRequest({ entityHash, mapDataHash, patch }: WEApplyPatchRequest) {
    if (!this.map.patches[mapDataHash]) {
      this.map.patches[mapDataHash] = {};
    }

    this.map.patches[mapDataHash][entityHash] = patch;
  }

  handleApplyAdditionChangeRequest(request: WEApplyAdditionChangeRequest) {
    const addition = this.map.additions[request.id];
    if (addition) {
      const setAdditionRequest: WESetAdditionRequest = {
        id: request.id,
        object: {
          ...addition,
          ...omit(request, 'id'),
        },
      };

      this.handleSetAdditionRequest(setAdditionRequest);
    }
  }

  handleSetAdditionRequest({ id, object }: WESetAdditionRequest) {
    this.map.additions[id] = object;

    if (object.grp !== -1) {
      if (!this.groupedAdditions[object.grp]) {
        this.groupedAdditions[object.grp] = {};
      }

      this.groupedAdditions[object.grp][id] = object;
    } else {
      this.ungroupedAdditions[id] = object;
    }
  }

  getGroupAdditions(grp: string): Record<string, WEMapAddition> {
    return this.groupedAdditions[grp] || {};
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

      sendGameClientEvent('we:deleteAddition', id);
      sendApiMessage(worldEditorApi.deleteAddition, {
        id
      } as WEDeleteAdditionRequest);
    }
  }

  setAdditionLabel(id: string, label: string) {
    const addition = this.map.additions[id];

    if (addition) {
      this.applyAdditionChange(id, {
        label,
      });
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

  createAdditionGroup(label: string) {
    const grp = fastRandomId();
    this.map.additionGroups[grp] = {
      label,
    };
    this.groupedAdditions[grp] = {};

    sendApiMessage(worldEditorApi.createAdditionGroup, {
      label,
    } as WECreateAdditionGroupRequest);
  }

  deleteAdditionGroup(grp: string, deleteAdditions: boolean) {
    const ids = Object.keys(this.groupedAdditions[grp] || {});

    delete this.map.additionGroups[grp];
    delete this.groupedAdditions[grp];

    if (deleteAdditions) {
      for (const id of ids) {
        delete this.map.additions[id];
      }

      sendGameClientEvent('we:deleteAdditions', JSON.stringify(ids));
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

    sendGameClientEvent('we:applyAdditionChange', JSON.stringify(request));
    sendApiMessage(worldEditorApi.applyAdditionChange, request);
  }
}
