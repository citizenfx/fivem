import { Api } from "fxdk/browser/Api";
import { ProjectApi } from "fxdk/project/common/project.api";
import { makeAutoObservable } from "mobx";
import { APIRQ } from "shared/api.requests";

export enum EntryRelocateOperation {
  None,
  Copy,
  Move,
}

export class RelocationContext {
  private sourceEntryPath: string | null = null;
  private operation = EntryRelocateOperation.None;

  get hasSourceEntryPath(): boolean {
    return !!this.sourceEntryPath;
  }

  constructor() {
    makeAutoObservable(this);
  }

  readonly getPasteMenuItemLabel = () => {
    if (!this.sourceEntryPath) {
      return 'Paste';
    }

    return `Paste "${this.sourceEntryPath}"`;
  };

  set(sourceEntryPath: string, operation: EntryRelocateOperation) {
    if (operation === EntryRelocateOperation.None) {
      throw new Error('Invalid relocation operation');
    }

    this.sourceEntryPath = sourceEntryPath;
    this.operation = operation;
  }

  reset() {
    this.sourceEntryPath = null;
    this.operation = EntryRelocateOperation.None;
  }

  apply(targetPath: string) {
    if (!this.sourceEntryPath || this.operation === EntryRelocateOperation.None) {
      this.sourceEntryPath = null;
      this.operation = EntryRelocateOperation.None;
      return;
    }

    const sourcePath = this.sourceEntryPath;
    const operation = this.operation;

    this.sourceEntryPath = null;
    this.operation = EntryRelocateOperation.None;

    switch (operation) {
      case EntryRelocateOperation.Move: {
        const request: APIRQ.MoveEntry = {
          sourcePath,
          targetPath,
        };

        return Api.send(ProjectApi.FsEndpoints.moveEntry, request);
      }

      case EntryRelocateOperation.Copy: {
        const request: APIRQ.CopyEntry = {
          sourcePath,
          targetPath,
        };

        return Api.send(ProjectApi.FsEndpoints.copyEntry, request);
      }
    }
  }
}
