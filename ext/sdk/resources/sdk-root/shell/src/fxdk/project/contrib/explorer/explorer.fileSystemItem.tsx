import React from 'react';
import { dispose, Disposer, IDisposable } from "fxdk/base/disposable";
import { convertMenuItemsToContextMenuItems, IMenuItem } from "fxdk/base/menu";
import { ContextMenuItemsCollection } from "fxdk/ui/controls/ContextMenu/ContextMenu";
import { checkedIcon, deleteIcon, openInExplorerIcon, renameIcon, searchIcon, uncheckedIcon } from "fxdk/ui/icons";
import { IShellCommandId, ShellCommands } from "shell-api/commands";
import { OpenFlag } from "store/generic/OpenFlag";
import { ProjectCommands } from "../../browser/project.commands";
import { copyItemIntoEntryFromDrop, renameFileSystemEntry } from "./basics";
import { IExplorerItemCreator, IInlineExplorerItemCreator } from "./explorer.itemCreate";
import { ProjectExplorerItemMenuGroups } from "./explorer.itemMenu";
import { ProjectExplorerParticipants } from "./projectExplorerExtensions";
import { IProjectExplorer, IProjectExplorerItem } from "./projectExplorerItem";
import { InlineItemCreator } from './ui/InlineItemCreator/InlineItemCreator';
import { convertItemCreatorToMenuItem } from './itemCreatorToMenuItem';
import { IFsEntry } from 'fxdk/project/common/project.types';
import { Project } from 'fxdk/project/browser/state/project';
import { AssetRuntimeData } from 'fxdk/project/browser/state/primitives/AssetRuntimeData';
import { FXCodeState } from 'personalities/fxcode/FXCodeState';

export abstract class ProjectExplorerFileSystemItem<TRuntimeData = any> implements IProjectExplorerItem {
  readonly id: string;
  readonly toDispose = new Disposer();

  protected context: IProjectExplorer.ItemContext;

  private _renamer?: IProjectExplorer.ItemRenamer;
  private _renamerComputed = false;

  private _inlineItemCreatorState = new OpenFlag(false);
  private _inlineItemCreator: IInlineExplorerItemCreator | null = null;

  private _runtimeData: AssetRuntimeData<TRuntimeData> | null = null;
  get runtimeData(): TRuntimeData | undefined {
    if (this._runtimeData === null) {
      this._runtimeData = new AssetRuntimeData<TRuntimeData>(this.entryPath);
    }

    return this._runtimeData.get();
  }

  private readonly _entry: Readonly<IFsEntry>;
  get entry() {
    return this._entry;
  }

  private readonly _entryPath: string;
  get entryPath(): string {
    return this._entryPath;
  }

  protected constructor(private definition: ProjectExplorerFileSystemItemTypes.Definition) {
    this.id = definition.entryPath;
    this._entry = definition.entry;
    this._entryPath = definition.entryPath;
  }

  abstract getDefaultIcon(expanded: boolean): React.ReactNode;
  abstract getIcon(): IProjectExplorer.ItemIcon;
  abstract getLabel(): IProjectExplorer.ItemLabel;

  dispose() {
    dispose(this.toDispose);
  }

  acceptParentContext(ownContext: IProjectExplorer.ItemContext) {
    this.context = ownContext;

    return this.context;
  }

  getContext() {
    return this.context;
  }

  getState() {
    if (this.definition.options?.notAnAsset) {
      return;
    }

    const assetConfig = Project.getAssetConfig(this.entryPath);

    return {
      enabled: assetConfig.enabled,
    };
  }

  renderCreator() {
    const creator = this._inlineItemCreator;

    if (!this._inlineItemCreatorState.isOpen || !creator) {
      return null;
    }

    return (
      <InlineItemCreator
        close={this._inlineItemCreatorState.close}
        creator={creator}
        basePath={this.entryPath}
      />
    );
  }

  protected register<T extends IDisposable>(disposable: T): T {
    return this.toDispose.register(disposable);
  }

  public computeRenamer(disposer: Disposer): IProjectExplorer.ItemRenamer | undefined {
    if (this._renamerComputed) {
      return this._renamer;
    }

    this._renamerComputed = true;

    const renameConfig = this.definition.rename;
    if (!renameConfig) {
      return;
    }

    const state = new OpenFlag(false);
    const entryPath = this.entryPath;

    return this._renamer = {
      command: disposer.register(ShellCommands.registerDynamic('fxdk.project.explorer.item.showRenamer', state.open)),
      state,

      validate: getValidate(renameConfig.validator),
      handleRename: renameConfig.handleRename || ((name: string) => renameFileSystemEntry(entryPath, name)),
    };
  }

  public computeMenuItems(disposer: Disposer): ContextMenuItemsCollection {
    if (!this._renamerComputed) {
      this.computeRenamer(disposer);
    }

    const options = this.definition.options;
    const entryPath = this.entryPath;

    const items: IMenuItem[] = (this.definition.menuItems || []).map((menuItem) => ({
      ...menuItem,
      group: menuItem.group || ProjectExplorerItemMenuGroups.EXTRAS,
    }));

    this.addItemCreatorsMenuItems(items);

    // Enable rename menu
    if (this._renamer) {
      items.push({
        id: 'rename',
        order: Number.MAX_SAFE_INTEGER - 1,
        group: ProjectExplorerItemMenuGroups.FSOPS,
        icon: renameIcon,
        label: 'Rename',
        commandId: this._renamer.command.id,
      });
    }

    items.push({
      id: 'find-in-folder',
      label: 'Find in Folder...',
      icon: searchIcon,
      order: Number.MIN_SAFE_INTEGER,
      group: ProjectExplorerItemMenuGroups.SEARCH,
      disabled: () => !FXCodeState.isReady,
      visible: () => this.entry.isDirectory,
      commandId: ProjectCommands.FIND_IN_FOLDER,
      commandArgs: () => [entryPath],
    });

    items.push({
      id: 'reveal-in-terminal',
      label: 'Open in Integrated Terminal',
      group: ProjectExplorerItemMenuGroups.SEARCH,
      disabled: () => !FXCodeState.isReady,
      visible: () => this.entry.isDirectory,
      commandId: ProjectCommands.REVEAL_IN_TERMINAL,
      commandArgs: () => [entryPath],
    });

    // Enable asset menu
    if (!options?.notAnAsset) {
      items.push({
        id: 'asset-enablement-enable',
        label: 'Enable',
        icon: uncheckedIcon,
        order: Number.MIN_SAFE_INTEGER,
        group: ProjectExplorerItemMenuGroups.ASSET,
        visible: () => !getAssetEnabled(entryPath),
        commandId: ProjectCommands.ASSET_SET_ENABLED,
        commandArgs: () => [entryPath, true],
      }, {
        id: 'asset-enablement-disable',
        label: 'Disable',
        icon: checkedIcon,
        order: Number.MIN_SAFE_INTEGER,
        group: ProjectExplorerItemMenuGroups.ASSET,
        visible: () => getAssetEnabled(entryPath),
        commandId: ProjectCommands.ASSET_SET_ENABLED,
        commandArgs: () => [entryPath, false],
      });
    }

    // Enable relocation source menu
    if (!options?.noRelocationSource) {
      items.push({
        id: 'relocation-copy',
        label: 'Copy',
        order: 1,
        group: ProjectExplorerItemMenuGroups.RELOCATE,
        commandId: ProjectCommands.SET_COPY_RELOCATION_CONTEXT,
        commandArgs: () => [entryPath],
      }, {
        id: 'relocation-move',
        label: 'Cut',
        order: 2,
        group: ProjectExplorerItemMenuGroups.RELOCATE,
        commandId: ProjectCommands.SET_MOVE_RELOCATION_CONTEXT,
        commandArgs: () => [entryPath],
      });
    }

    // Enable relocation target menu
    if (!options?.noRelocationTarget) {
      items.push({
        id: 'relocation-paste',
        order: 3,
        group: ProjectExplorerItemMenuGroups.RELOCATE,
        label: Project.relocation.getPasteMenuItemLabel,
        disabled: () => !Project.relocation.hasSourceEntryPath,
        commandId: ProjectCommands.APPLY_RELOCATION,
        commandArgs: () => [entryPath],
      });
    }

    // Enabled delete menu
    if (!options?.noDefaultDelete) {
      items.push({
        id: 'delete',
        order: Number.MAX_SAFE_INTEGER,
        group: ProjectExplorerItemMenuGroups.FSOPS,
        icon: deleteIcon,
        label: 'Delete',
        commandId: ProjectCommands.DELETE_ENTRY,
        commandArgs: () => [entryPath],
      });
    }

    // Enable open-in-explorer menu
    if (!options?.noOpenInExplorer) {
      items.push({
        id: 'open-in-explorer',
        icon: openInExplorerIcon,
        label: 'Open in Explorer',
        group: ProjectExplorerItemMenuGroups.MISC,
        order: Number.MAX_SAFE_INTEGER,
        commandId: ProjectCommands.OPEN_IN_EXPLORER,
        commandArgs: () => [entryPath],
      });
    }

    return convertMenuItemsToContextMenuItems(items);
  }

  public computeDragAndDrop(): IProjectExplorer.ItemDragAndDrop | undefined {
    const entryPath = this.entryPath;

    const dnd = this.definition.dragAndDrop;
    if (!dnd) {
      return;
    }

    const computedDnd: IProjectExplorer.ItemDragAndDrop = {
      drag: dnd.drag,
    };

    if (dnd.drop) {
      computedDnd.drop = {
        ...dnd.drop,
        handleDrop: dnd.drop.handleDrop || ((item) => copyItemIntoEntryFromDrop(entryPath, item as any)),
      };
    }

    return computedDnd;
  }

  protected addItemCreatorsMenuItems(items: IMenuItem[]) {
    if (this.definition.options?.noFileSystemChildren) {
      return;
    }

    if (!this.entry.isDirectory) {
      return;
    }

    items.push(...this.computeItemCreatorsMenuItems());
  }

  protected computeItemCreatorsMenuItems(): IMenuItem[] {
    const context = this.getContext();

    return ProjectExplorerParticipants.getAllItemCreators().map((creator) => {
      return convertItemCreatorToMenuItem(creator, this.getItemCreatorCommandId(creator), context);
    });
  }

  private getItemCreatorCommandId(creator: IExplorerItemCreator): IShellCommandId {
    switch (creator.type) {
      case 'inline': {
        return this.register(ShellCommands.registerDynamic(`showInlineItemCreator(${creator.id})`, () => {
          Project.setPathState(this.entryPath, true);
          this._inlineItemCreatorState.open();
          this._inlineItemCreator = creator;
        })).id;
      }

      case 'custom': {
        return this.register(ShellCommands.registerDynamic(creator.openCommandId, () => {
          Project.setPathState(this.entryPath, true);
          ShellCommands.invoke(creator.openCommandId, this.entryPath);
        })).id;
      }
    }
  }
}

function getAssetEnabled(assetPath: string): boolean {
  return !!Project.getAssetConfig(assetPath).enabled;
}

function getValidate(validator: ProjectExplorerFileSystemItemTypes.RenameValidator): IProjectExplorer.ItemRenamer['validate'] {
  if (typeof validator === 'function') {
    return validator;
  }

  return (name: string) => validator.test(name);
}

export namespace ProjectExplorerFileSystemItemTypes {
  export type RenameValidator =
    | RegExp
    | ((name: string) => boolean);

  export interface Options {
    noFileSystemChildren?: boolean,
    noOpenInExplorer?: boolean,
    noDefaultDelete?: boolean,
    notAnAsset?: boolean,
    noRelocationSource?: boolean,
    noRelocationTarget?: boolean,
    noCreateChild?: boolean,
  }

  export interface Definition {
    entry: IFsEntry,
    entryPath: string,
    rename?: {
      validator: RenameValidator,
      handleRename?(newName: string): void;
    },
    options?: Options,
    menuItems?: IMenuItem[],
    dragAndDrop?: {
      drag?: IProjectExplorer.ItemDrag,

      /**
       * Only acceptTypes is required as filesystem item provides default handleDrop - copy behaviour
       */
      drop?: Partial<IProjectExplorer.ItemDrop> & Required<Pick<IProjectExplorer.ItemDrop, 'acceptTypes'>>,
    },
  }
}
