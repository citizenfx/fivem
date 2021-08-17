import { WEEntityMatrix, WEMapPatch } from "@sdk-root/backend/world-editor/world-editor-types";

const activeMapdatas: Record<number, boolean> = {};
let mapDataLoaded: (mapdata: number) => void = () => {};

on('mapDataLoaded', (mapdata: number) => {
  activeMapdatas[mapdata] = true;

  mapDataLoaded(mapdata);
});
on('mapDataUnloaded', (mapdata: number) => {
  delete activeMapdatas[mapdata];
});

export enum UpdateOrCreateResult {
  UPDATE,
  CREATE,
  FAIL,
}

export class PatchManager {
  constructor(
    private patches: Record<number, Record<number, WEMapPatch>>,
  ) {
    mapDataLoaded = this.handleMapdataLoaded;

    for (const mapdataString of Object.keys(activeMapdatas)) {
      this.handleMapdataLoaded(parseInt(mapdataString, 10));
    }
  }

  isMapdataLoaded(mapdata: string | number): boolean {
    return !!activeMapdatas[mapdata];
  }

  get(mapdata: string | number, entity: string | number): WEMapPatch | void {
    return this.patches[mapdata]?.[entity];
  }

  getHandle(mapdata: string | number, entity: string | number): number | null {
    const [success, handle] = GetMapdataEntityHandle(toNumber(mapdata), toNumber(entity));
    if (success) {
      return handle;
    }

    return null;
  }

  set(mapdata: string | number, entity: string | number, patch: WEMapPatch) {
    this.patches[mapdata] ??= {};
    this.patches[mapdata][entity] = patch;

    applyMapdataPatch(toNumber(mapdata), toNumber(entity), patch.mat);
  }

  updateMat(mapdata: string | number, entity: string | number, mat: WEEntityMatrix) {
    const patch = this.get(mapdata, entity);
    if (patch) {
      patch.mat = mat;

      applyMapdataPatch(toNumber(mapdata), toNumber(entity), patch.mat);
    }
  }

  updateOrCreate(entityGuid: number, mat: WEEntityMatrix): [UpdateOrCreateResult, number, number] {
    let result = UpdateOrCreateResult.FAIL;

    const [success, mapdata, entity] = GetEntityMapdataOwner(entityGuid);
    if (success) {
      let patch = this.get(mapdata, entity);

      if (patch) {
        result = UpdateOrCreateResult.UPDATE;

        patch.mat = mat;
      } else {
        result = UpdateOrCreateResult.CREATE;

        this.patches[mapdata] ??= {};
        this.patches[mapdata][entity] = patch = {
          cam: [0,0,0,0,0,0],
          label: '',
          mat,
        };
      }

      applyMapdataPatch(mapdata, entity, patch.mat);
    }

    return [result, mapdata, entity];
  }

  delete(mapdata: string | number, entity: string | number) {
    const patch = this.get(mapdata, entity);
    if (patch) {
      delete this.patches[mapdata][entity];

      ResetMapdataEntityMatrix(toNumber(mapdata), toNumber(entity));
    }
  }

  private readonly handleMapdataLoaded = (mapdata: number) => {
    for (const [entityString, patch] of Object.entries(this.patches[mapdata] || {})) {
      applyMapdataPatch(mapdata, parseInt(entityString, 10), patch.mat);
    }
  }
}

function applyMapdataPatch(mapdata: number, entity: number, mat: Float32Array | number[]) {
  if (!activeMapdatas[mapdata]) {
    return;
  }

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

function toNumber(n: string | number): number {
  if (typeof n === 'string') {
    return parseInt(n, 10);
  }

  return n;
}
