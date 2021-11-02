import { AssocRegistry } from "fxdk/base/registry";
import { IProjectExplorerItem } from "./projectExplorerItem";
import { FileExplorerItemHandler, FileExplorerItemCreator } from "./items/file";
import { DirectoryExplorerItemCreator, DirectoryExplorerItemHandler } from "./items/directory";
import { IExplorerItemCreator } from "./explorer.itemCreate";
import { IFsEntry } from "fxdk/project/common/project.types";

type IProjectExplorerItemHandler = new(entry: IFsEntry, entryPath: string) => IProjectExplorerItem;

export const ProjectExplorerParticipants = new class ProjectExplorerParticipants {
  private handlers = new AssocRegistry<IProjectExplorerItemHandler>('project-explorer-items', true);
  private handlersOverrides: string[] = [];

  private itemCreators = new AssocRegistry<IExplorerItemCreator, string>('explorer-item-creators', true);

  constructor() {
    this.registerHandler('file', FileExplorerItemHandler);
    this.registerItemCreator(FileExplorerItemCreator);

    this.registerHandler('directory', DirectoryExplorerItemHandler);
    this.registerItemCreator(DirectoryExplorerItemCreator);
  }

  overrideHandler(handle: string, handler: IProjectExplorerItemHandler) {
    if (this.handlersOverrides.includes(handle)) {
      throw new Error(`Handle "${handle}" has already been overriden`);
    }

    this.handlersOverrides.push(handle);
    this.handlers.register(handle, handler);
  }

  registerHandler(handle: string, handler: IProjectExplorerItemHandler) {
    if (this.handlers.has(handle)) {
      throw new Error(`Handle "${handle}" already has handler, if you want to override - use ProjectExplorerParticipants.overrideHandler`);
    }

    this.handlers.register(handle, handler);
  }

  getHandlerForEntry(entry: IFsEntry): IProjectExplorerItemHandler {
    const ctor = this.handlers.get(entry.handle);
    if (!ctor) {
      throw new Error(`No handler "${entry.handle}" registered`);
    }

    return ctor;
  }

  registerItemCreator(creator: IExplorerItemCreator) {
    this.itemCreators.register(creator.id, creator);
  }

  getItemCreator(id: string): IExplorerItemCreator | undefined {
    return this.itemCreators.get(id);
  }

  getAllItemCreators(): ReadonlyArray<IExplorerItemCreator> {
    return [...this.itemCreators.getAll()];
  }
}();
