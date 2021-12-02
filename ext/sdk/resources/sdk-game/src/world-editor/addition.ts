import { WEEntityMatrix, WEEntityMatrixIndex } from "@sdk-root/backend/world-editor/world-editor-types";
import { joaatUint32 } from "../shared";
import { applyAdditionMatrix } from "./math";
import { Sector, SectorId } from "./sector";

const modelHashCache: Record<number | string, number> = {};
const modelRefCounter: Record<number, number> = {};

function getModel(mdl: string | number): number {
  if (!(mdl in modelHashCache)) {
    modelHashCache[mdl] = typeof mdl === 'string'
      ? joaatUint32(mdl)
      : mdl;
  }

  return modelHashCache[mdl];
}

enum AdditionState {
  NotLoaded,
  Loading,
  Loaded,
  Error,
}

export class Addition {
  private state = AdditionState.NotLoaded;

  handle: number | undefined;
  readonly model: number;

  get isLoaded(): boolean {
    return this.state === AdditionState.Loaded;
  }

  get isNotLoaded(): boolean {
    return this.state === AdditionState.NotLoaded;
  }

  get isLoading(): boolean {
    return this.state === AdditionState.Loading;
  }

  get position(): [number, number, number] {
    return [
      this.matrix[WEEntityMatrixIndex.AX],
      this.matrix[WEEntityMatrixIndex.AY],
      this.matrix[WEEntityMatrixIndex.AZ],
    ];
  }

  get sectorId(): SectorId {
    return Sector.idFromCoords(
      this.matrix[WEEntityMatrixIndex.AX],
      this.matrix[WEEntityMatrixIndex.AY],
    );
  }

  constructor(
    public readonly id: string,
    model: string | number,
    public matrix: WEEntityMatrix,
  ) {
    this.model = getModel(model);

    modelRefCounter[this.model] ??= 0;
    modelRefCounter[this.model] += 1;
  }

  private reqs = 0;

  toString(): string {
    return `${this.id}, state: ${AdditionState[this.state]}, handle: ${this.handle}, model: ${this.model}, in cd image: ${IsModelInCdimage(this.model)}, reqs: ${this.reqs}`;
  }

  dispose() {
    if (modelRefCounter[this.model]) {
      modelRefCounter[this.model] -= 1;

      if (modelRefCounter[this.model] === 0) {
        SetModelAsNoLongerNeeded(this.model);
      }
    }

    if (this.isLoaded) {
      DeleteObject(this.handle);
    }
  }

  update() {
    if (this.isLoading) {
      this.tryTransitionToLoaded();
    }
  }

  load() {
    if (!this.isNotLoaded) {
      return;
    }

    this.state = AdditionState.Loading;

    RequestModel(this.model);
    this.reqs++;

    // Could be already loaded, right
    this.tryTransitionToLoaded();
  }

  unload() {
    if (this.isNotLoaded) {
      return;
    }

    if (this.isLoaded) {
      DeleteObject(this.handle);
    }

    this.state = AdditionState.NotLoaded;
    this.handle = undefined;
  }

  updateMatrix(matrix: Float32Array | WEEntityMatrix) {
    this.matrix = Array.from(matrix) as WEEntityMatrix;

    applyAdditionMatrix(this.handle, this.matrix);
  }

  private tryTransitionToLoaded() {
    if (!HasModelLoaded(this.model)) {
      RequestModel(this.model);
      this.reqs++;
      return;
    }

    this.state = AdditionState.Loaded;

    this.createObject();
  }

  private createObject() {
    const [x, y, z] = this.position;

    this.handle = CreateObject(this.model, x, y, z, false, false, false);

    FreezeEntityPosition(this.handle, true);

    applyAdditionMatrix(this.handle, this.matrix);
  }
}
