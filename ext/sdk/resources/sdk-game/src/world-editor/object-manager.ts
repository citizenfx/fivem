import { joaatUint32 } from "../shared";
import { WEMapAddition } from "./map-types";

const mdlHashCache = {};

export class ObjectManager {
  private handles: Record<string, number> = {};
  private handleToObjectIdMap: Record<number, string> = {};

  constructor(
    private objects: Record<string, WEMapAddition>,
  ) {
    Object.keys(objects).forEach((objectId) => this.handles[objectId] = 0xFFFFFFFF);
  }

  isAddition(handle: number): boolean {
    return handle in this.handleToObjectIdMap;
  }

  getObjectId(handle: number): string | void {
    return this.handleToObjectIdMap[handle];
  }

  set(objectId: string, object: WEMapAddition) {
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
      const { mdl, mat } = this.objects[objectId];

      if (!(mdl in mdlHashCache)) {
        mdlHashCache[mdl] = typeof mdl === 'string'
          ? joaatUint32(mdl)
          : mdl;
      }
      const mdlHash = mdlHashCache[mdl];

      switch (handle) {
        case 0xFFFFFFFF: {
          RequestModel(mdlHash);
          this.handles[objectId] = 0;
          break;
        }

        case 0: {
          if (HasModelLoaded(mdlHash)) {
            const handle = CreateObject(mdlHash, mat[12], mat[13], mat[14], false, false, false);

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
