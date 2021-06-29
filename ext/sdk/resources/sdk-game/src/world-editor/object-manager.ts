import { WorldEditorMapObject } from "./map-types";

export class ObjectManager {
  private handles: Record<string, number> = {};
  private handleToObjectIdMap: Record<number, string> = {};

  constructor(
    private objects: Record<string, WorldEditorMapObject>,
  ) {
    Object.keys(objects).forEach((objectId) => this.handles[objectId] = 0xFFFFFFFF);
  }

  isAddition(handle: number): boolean {
    return handle in this.handleToObjectIdMap;
  }

  getObjectId(handle: number): string | void {
    return this.handleToObjectIdMap[handle];
  }

  set(objectId: string, object: WorldEditorMapObject) {
    this.objects[objectId] = object;

    if (!(objectId in this.handles)) {
      this.handles[objectId] = 0xFFFFFFFF;
    }
  }

  delete(objectId: string) {
    const handle = this.handles[objectId];
    delete this.handleToObjectIdMap[handle];

    if (handle !== 0 && DoesEntityExist(handle)) {
      DeleteObject(handle);
    }

    delete this.objects[objectId];
    delete this.handles[objectId];
  }

  update() {
    for (const [objectId, handle] of Object.entries(this.handles)) {
      const { hash, mat } = this.objects[objectId];

      switch (handle) {
        case 0xFFFFFFFF: {
          RequestModel(hash);
          this.handles[objectId] = 0;
          break;
        }

        case 0: {
          if (HasModelLoaded(hash)) {
            const handle = CreateObject(hash, mat[12], mat[13], mat[14], false, false, false);

            SetEntityMatrix(
              handle,
              mat[4], mat[5], mat[6], // right
              mat[0], mat[1], mat[2], // forward
              mat[8], mat[9], mat[10], // up
              mat[12], mat[13], mat[14], // at
            );

            this.handles[objectId] = handle;
            this.handleToObjectIdMap[handle] = objectId;
            // SetModelAsNoLongerNeeded(hash);
          }
          break;
        }

        default: {
          // if (DoesEntityExist(handle)) {
          //   this.handles[objectId] = 0xFFFFFFFF;
          // }
        }
      }
    }
  }
}
