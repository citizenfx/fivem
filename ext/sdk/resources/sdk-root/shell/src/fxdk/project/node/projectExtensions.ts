import { AssetRuntime } from "fxdk/project/node/asset/asset-runtime";
import { FsWatcherEvent } from "backend/fs/fs-watcher";
import { IFsEntry } from "../common/project.types";

export interface IFsEntryHandler {
  handles(entry: IFsEntry, entryPath: string): Promise<boolean> | boolean;

  expandEvent?(event: FsWatcherEvent): FsWatcherEvent[] | undefined;

  spawnAssetRuntime?(fsEntry: IFsEntry, fsEntryPath: string): AssetRuntime;
}

type IFsHandlerCtor = new() => IFsEntryHandler;

export const ProjectFsExtensions = new class ProjectFsExtensions {
  private registry = new Map<string, IFsHandlerCtor>();
  private resolvedHandlers = new Map<string, IFsEntryHandler>();

  register(handle: string, handler: IFsHandlerCtor) {
    if (this.registry.has(handle)) {
      throw new Error(`Unable to reassign handle "${handle}"`);
    }

    this.registry.set(handle, handler);
  }

  getHandler(entry: IFsEntry): IFsEntryHandler | undefined {
    return this.getResolvedHandler(entry.handle);
  }

  async getHandle(entry: IFsEntry, entryPath: string): Promise<string> {
    for (const handle of this.registry.keys()) {
      if (await this.getResolvedHandler(handle).handles(entry, entryPath)) {
        return handle;
      }
    }

    return entry.isDirectory
      ? 'directory'
      : 'file';
  }

  expandEvent(event: FsWatcherEvent): FsWatcherEvent[] {
    let extraEvents: FsWatcherEvent[][] = [];

    for (const handle of this.registry.keys()) {
      const handlerEvents = this.getResolvedHandler(handle).expandEvent?.(event);
      if (handlerEvents?.length) {
        extraEvents.push(handlerEvents);
      }
    }

    return extraEvents.flat();
  }

  private getResolvedHandler(handle: string): IFsEntryHandler {
    let handler = this.resolvedHandlers.get(handle);
    if (!handler) {
      const handlerCtor = this.registry.get(handle);
      if (!handlerCtor) {
        throw new Error(`No handler for "${handle}"`);
      }

      handler = new handlerCtor();
    }

    return handler;
  }
}();
