import { FsUpdateKind, IFsEntry, IFsUpdate } from "fxdk/project/common/project.types";
import { dispose, Disposer, IDisposableObject } from "fxdk/base/disposable";
import { Api } from "fxdk/browser/Api";
import { makeAutoObservable } from "mobx";
import { ProjectApi } from "fxdk/project/common/project.api";

export class ProjectFsState implements IDisposableObject {
  private toDispose: Disposer;

  constructor(public rootPath: string, public fsEntry: IFsEntry) {
    makeAutoObservable(this);

    this.toDispose = new Disposer();

    this.toDispose.register(Api.on(ProjectApi.FsEndpoints.update, this.handleUpdate));
  }

  dispose() {
    dispose(this.toDispose);
  }

  getRootChildren(): IFsEntry[] {
    return Object.values(this.fsEntry.children);
  }

  getChildren(path: string[]): IFsEntry[] {
    return Object.values(this.getFsEntry(path)?.children || {});
  }

  private readonly handleUpdate = (update: IFsUpdate) => {
    switch (update.kind) {
      case FsUpdateKind.Set: {
        const parentFsEntry = this.getFsEntry(update.parentPath);
        if (parentFsEntry) {
          parentFsEntry.children[update.fsEntry.name] = update.fsEntry;
        } else {
          console.error('No parent fs entry for update', update);
        }
        break;
      }
      case FsUpdateKind.Update: {
        const fsEntry = this.getFsEntry(update.path);
        if (fsEntry) {
          Object.assign(fsEntry, update.fsEntry);
        } else {
          console.error('No fs entry for update', update);
        }
        break;
      }
      case FsUpdateKind.Rename: {
        const oldParentFsEntry = this.getFsEntry(update.oldParentPath);
        const newParentFsEntry = this.getFsEntry(update.newParentPath);

        if (!oldParentFsEntry) {
          console.error('Unable to perform rename, no parent fs entry', { update, oldParentFsEntry, newParentFsEntry });
          break;
        }

        const oldFsEntry = oldParentFsEntry.children[update.oldName];
        delete oldParentFsEntry.children[update.oldName];

        if (oldFsEntry && newParentFsEntry) {
          oldFsEntry.name = update.newName;
          newParentFsEntry.children[update.newName] = oldFsEntry;
        } else {
          console.error('Unable to perform rename', { update, oldParentFsEntry, newParentFsEntry, oldFsEntry });
        }

        break;
      }
      case FsUpdateKind.Delete: {
        const parentFsEntry = this.getFsEntry(update.parentPath);
        if (parentFsEntry) {
          delete parentFsEntry.children[update.name];
        } else {
          console.error('No parent fs entry for update', update);
        }
      }
    }
  };

  private getFsEntry(path: string[]): IFsEntry | null {
    return path.reduce((fsEntry, pathPart) => fsEntry?.children[pathPart] || null, this.fsEntry);
  }
}
