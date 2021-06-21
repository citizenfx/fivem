import { WorldEditorApplyAdditionChangeRequest, WorldEditorApplyPatchRequest, WorldEditorCam, WorldEditorDeleteAdditionRequest, WorldEditorMap, WorldEditorMapObject, WorldEditorPatch, WorldEditorSetAdditionRequest } from "backend/world-editor/world-editor-types";
import { makeAutoObservable } from "mobx";
import { worldEditorApi } from "shared/api.events";
import { sendApiMessage } from "utils/api";
import { omit } from "utils/omit";

export class WorldEditorMapState {
  private ungroupedAdditions: Record<string, WorldEditorMapObject> = {};
  private groupedAdditions: Record<number, Record<string, WorldEditorMapObject>> = {};

  constructor(private map: WorldEditorMap) {
    makeAutoObservable(this);

    for (const [additionId, addition] of Object.entries(map.additions)) {
      if (addition.grp !== -1) {
        if (!this.groupedAdditions[addition.grp]) {
          this.groupedAdditions[addition.grp] = {};
        }

        this.groupedAdditions[addition.grp][additionId] = addition;
      } else {
        this.ungroupedAdditions[additionId] = addition;
      }
    }
  }

  get cam(): WorldEditorCam {
    return this.map.meta.cam;
  }

  get additions(): Record<string, WorldEditorMapObject> {
    return this.map.additions;
  }

  get additionGroups(): string[] {
    return this.map.additionGroups;
  }

  get additionsByGroups(): Record<number, Record<string, WorldEditorMapObject>> {
    return this.groupedAdditions;
  }

  get additionsUngrouped(): Record<string, WorldEditorMapObject> {
    return this.ungroupedAdditions;
  }

  get patches(): Record<string, Record<string, WorldEditorPatch>> {
    return this.map.patches;
  }

  handleApplyPatchRequest({ entityHash, mapDataHash, patch }: WorldEditorApplyPatchRequest) {
    if (!this.map.patches[mapDataHash]) {
      this.map.patches[mapDataHash] = {};
    }

    this.map.patches[mapDataHash][entityHash] = patch;
  }

  handleApplyAdditionChangeRequest(request: WorldEditorApplyAdditionChangeRequest) {
    const addition = this.map.additions[request.id];
    if (addition) {
      const setAdditionRequest: WorldEditorSetAdditionRequest = {
        id: request.id,
        object: {
          ...addition,
          ...omit(request, 'id'),
        },
      };

      this.handleSetAdditionRequest(setAdditionRequest);
    }
  }

  handleSetAdditionRequest({ id, object }: WorldEditorSetAdditionRequest) {
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

  deleteAddition(id: string) {
    const addition = this.map.additions[id];
    if (addition) {
      if (addition.grp !== -1 && this.groupedAdditions[addition.grp]) {
        delete this.groupedAdditions[addition.grp][id];
      } else {
        delete this.ungroupedAdditions[id];
      }

      delete this.map.additions[id];

      sendGameClientEvent('we:deleteAddition', id);
      sendApiMessage(worldEditorApi.deleteAddition, {
        id
      } as WorldEditorDeleteAdditionRequest);
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

  setAdditionGroup(id: string, grp: number) {
    const addition = this.map.additions[id];
    if (addition) {
      if (addition.grp === -1) {
        delete this.ungroupedAdditions[id];
      } else {
        delete this.groupedAdditions[addition.grp];
      }

      addition.grp = grp;

      if (grp === -1) {
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

  private applyAdditionChange(id: string, change: Partial<WorldEditorMapObject>) {
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

  private broadcastAdditionChange(id: string, change: Partial<WorldEditorMapObject>) {
    const request: WorldEditorApplyAdditionChangeRequest = {
      id,
      ...change,
    };

    sendGameClientEvent('we:applyAdditionChange', JSON.stringify(request));
    sendApiMessage(worldEditorApi.applyAdditionChange, request);
  }
}
