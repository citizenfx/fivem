import { joaatUint32 } from "../shared";
import { CameraController } from "./camera-controller";
import { applyEntityMatrix, applyScale, makeEntityMatrix, vectorLength } from "./math";

function getObjectPosition() {
  const fw = CameraController.getForwardVector().copy().mult(5);

  return CameraController.getPosition().copy().add(fw);
}

function getSize(min: number[], max: number[]): number {
  return vectorLength([
    max[0] - min[0],
    max[1] - min[1],
    max[2] - min[2],
  ]);
}

export const PreviewManager = new class PreviewManager {
  private currObjectNameHash = 0;
  private currObjectName = '';
  private nextObjectName = '';

  private rotation = 0;

  private objectHandle: number | void;

  constructor() {
    on('setPreviewObjectName', (objectName: string) => {
      this.nextObjectName = objectName;
    });
  }

  update() {
    if (this.nextObjectName !== this.currObjectName) {
      if (this.objectHandle) {
        DeleteObject(this.objectHandle);
        this.objectHandle = undefined;
      }

      this.currObjectName = this.nextObjectName;

      if (this.currObjectName) {
        this.currObjectNameHash = joaatUint32(this.currObjectName);
        this.rotation = 0;

        RequestModel(this.currObjectNameHash);
      }
    }

    if (!this.currObjectName) {
      return;
    }

    if (!this.objectHandle) {
      if (HasModelLoaded(this.currObjectNameHash)) {
        const pos = getObjectPosition();
        this.objectHandle = CreateObject(this.currObjectNameHash, pos.x, pos.y, pos.z, false, false, false);
        SetModelAsNoLongerNeeded(this.currObjectNameHash);
      }

      return;
    }

    this.updateRotation();

    const pos = getObjectPosition();
    const [min, max] = GetModelDimensions(this.currObjectNameHash);

    SetEntityRotation(this.objectHandle, 0, 0, this.rotation, 2, false);

    const mat = makeEntityMatrix(this.objectHandle);
    const size = getSize(min, max);
    const adjustedSize = 3/size;

    const zOffset = ((min[2] * adjustedSize)/2) + ((max[2] * adjustedSize)/2);

    applyScale(mat, [adjustedSize, adjustedSize, adjustedSize]);

    applyEntityMatrix(this.objectHandle, mat);

    SetEntityCoords(this.objectHandle, pos.x, pos.y, pos.z - zOffset, false, false, false, false);
  }

  private updateRotation() {
    this.rotation += GetGameTimer() * 0.0000005;

    if (this.rotation >= 360) {
      this.rotation = 0;
    }
  }
};
