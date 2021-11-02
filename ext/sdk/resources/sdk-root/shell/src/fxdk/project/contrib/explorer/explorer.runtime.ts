import React from 'react';
import { computed } from 'mobx';
import { dispose, Disposer, IDisposableObject } from "fxdk/base/disposable";
import { IDynamicShellCommand, IShellCommandId, ShellCommands } from "shell-api/commands";
import { OpenFlag } from "store/generic/OpenFlag";
import { Api } from 'fxdk/browser/Api';
import { ContextMenuItemsCollection } from 'fxdk/ui/controls/ContextMenu/ContextMenu';
import { convertMenuItemsToContextMenuItems } from 'fxdk/base/menu';
import { projectSettingsIcon } from 'fxdk/ui/icons';
import { ProjectSettingsCommands } from 'fxdk/project/contrib/settings/settings.commands';
import { IFsEntry } from 'fxdk/project/common/project.types';
import { Project } from 'fxdk/project/browser/state/project';
import { IExplorerItemCreator, IInlineExplorerItemCreator } from './explorer.itemCreate';
import { IProjectExplorer, IProjectExplorerItem } from "./projectExplorerItem";
import { ProjectExplorerParticipants } from './projectExplorerExtensions';
import { convertItemCreatorToMenuItem } from './itemCreatorToMenuItem';
import { ProjectStateEvents } from 'fxdk/project/browser/state/projectStateEvents';
import { ProjectApi } from 'fxdk/project/common/project.api';

export const ExplorerRuntime = new class ExplorerRuntime {
  public rootInlineItemCreator: IInlineExplorerItemCreator | undefined;
  public rootInlineItemCreatorState = new OpenFlag(false);

  private toDispose = new Disposer();

  private items = new Map<string, RuntimeItem>();
  private itemsHandles = new Map<string, string>();
  private itemsCreatorsCommandIds = new Map<string, IShellCommandId>();

  constructor() {
    Api.on(ProjectApi.FsEndpoints.entryRenamed, ({ fromEntryPath }) => this.disposeItemById(fromEntryPath));
    Api.on(ProjectApi.FsEndpoints.entryDeleted, ({ entryPath }) => this.disposeItemById(entryPath));

    ProjectStateEvents.BeforeClose.addListener(() => this.reset());
  }

  getOrCreateFileSystemItem(entry: IFsEntry, entryPath: string, parent?: RuntimeItem): RuntimeItem {
    let runtimeItem = this.items.get(entryPath);
    if (runtimeItem) {
      if (this.itemsHandles.get(entryPath) === entry.handle) {
        return runtimeItem;
      }

      this.disposeItem(runtimeItem);
    }

    this.itemsHandles.set(entryPath, entry.handle);

    return this.createItem(this.createHandlerItemForEntry(entry, entryPath), parent);
  }

  getOrCreateItem(item: IProjectExplorerItem, parent?: RuntimeItem): RuntimeItem {
    let runtimeItem = this.items.get(item.id);
    if (runtimeItem) {
      return runtimeItem;
    }

    return this.createItem(item, parent);
  }

  getItem(id: string): RuntimeItem | undefined {
    return this.items.get(id);
  }

  getItemDefaultIcon(id: string, expanded = false): React.ReactNode {
    return this.items.get(id)?.item.getDefaultIcon(expanded) || null;
  }

  getOrCreateRootItemCreatorCommandId(itemCreator: IExplorerItemCreator): IShellCommandId {
    let commandId = this.itemsCreatorsCommandIds.get(itemCreator.id);
    if (!commandId) {
      switch (itemCreator.type) {
        case 'inline': {
          commandId = this.toDispose.register(ShellCommands.registerDynamic(itemCreator.id, () => {
            this.rootInlineItemCreatorState.open();
            this.rootInlineItemCreator = itemCreator;
          })).id;
          break;
        }
        case 'custom': {
          commandId = this.toDispose.register(ShellCommands.registerDynamic(itemCreator.id, () => {
            ShellCommands.invoke(itemCreator.openCommandId, Project.path);
          })).id;
          break;
        }
      }

      this.itemsCreatorsCommandIds.set(itemCreator.id, commandId);
    }

    return commandId;
  }

  getRootMenuItems() {
    return computed(() => convertMenuItemsToContextMenuItems([
      ...ProjectExplorerParticipants.getAllItemCreators().map((creator) => {
        return convertItemCreatorToMenuItem(creator, this.getOrCreateRootItemCreatorCommandId(creator));
      }),
      {
        id: 'settings',
        icon: projectSettingsIcon,
        label: 'Project Settings',
        commandId: ProjectSettingsCommands.OPEN,
      },
    ])).get();
  }

  private createHandlerItemForEntry(entry: IFsEntry, entryPath: string): IProjectExplorerItem {
    const ItemCtor = ProjectExplorerParticipants.getHandlerForEntry(entry);

    return new ItemCtor(entry, entryPath);
  }

  private createItem(item: IProjectExplorerItem, parent: RuntimeItem | undefined): RuntimeItem {
    const rt = new RuntimeItem(item, parent);

    this.items.set(rt.id, rt);

    return rt;
  }

  private disposeItemById(itemId: string) {
    const item = this.items.get(itemId);
    if (!item) {
      return;
    }

    this.disposeItem(item);
  }

  private disposeItem(item: RuntimeItem) {
    item.disposeDeep();
    this.items.delete(item.id);
    this.itemsHandles.delete(item.id);
  }

  private reset() {
    this.itemsCreatorsCommandIds.clear();

    dispose(this.toDispose);
    this.toDispose = new Disposer();

    for (const runtimeItem of this.items.values()) {
      dispose(runtimeItem);
    }

    this.items.clear();
  }
}();

export interface RuntimeItemRenaming {
  command: IDynamicShellCommand,
  state: OpenFlag,

  validate(name: string): boolean,
  handleRename(name: string): void,
}

export class RuntimeItem implements IDisposableObject {
  readonly id = this.item.id;
  readonly ref = React.createRef();

  readonly toDispose = new Disposer();

  rendered = false;
  depth = 0;

  readonly context: IProjectExplorer.ItemContext = this.item?.acceptParentContext?.(this.parent?.context || {}) || {};
  readonly renamer: Readonly<IProjectExplorer.ItemRenamer> | undefined = this.item.computeRenamer?.(this.toDispose);
  readonly menuItems: Readonly<ContextMenuItemsCollection> = this.item.computeMenuItems?.(this.toDispose) || [];
  readonly dragAndDrop: Readonly<IProjectExplorer.ItemDragAndDrop> | undefined = this.item.computeDragAndDrop?.(this.toDispose);

  readonly noChildren: boolean;
  readonly hasShouldRenderChildren = Boolean(this.item.shouldRenderChildren);
  readonly hasGetState = Boolean(this.item.getState);
  readonly hasGetChildren = Boolean(this.item.getChildren);
  readonly hasGetTitle = Boolean(this.item.getTitle);
  readonly hasGetStatus = Boolean(this.item.getStatus);
  readonly hasRenderInWrapper = Boolean(this.item.renderInWrapper);

  readonly selectionFlag = new OpenFlag(this.id === Project.selectedEntryPath);

  private children = new Set<RuntimeItem>();
  private pendingScrollIntoView = false;

  constructor(
    readonly item: IProjectExplorerItem,
    private parent: RuntimeItem | undefined,
  ) {
    if (parent) {
      this.depth = parent.depth + 1;
    }

    this.toDispose.add(item);

    const forcedNoChildren = this.item.forceNoChildren?.();

    if (forcedNoChildren === undefined) {
      this.noChildren = false;
    } else {
      this.noChildren = forcedNoChildren;
    }

    parent?.addChild(this);
  }

  addChild(child: RuntimeItem) {
    this.children.add(child);
  }

  deleteChild(child: RuntimeItem) {
    this.children.delete(child);
  }

  dispose() {
    dispose(this.toDispose);
  }

  disposeDeep() {
    this.parent?.deleteChild(this);

    for (const child of this.children) {
      dispose(child);
    }

    this.parent = undefined;
    this.children = new Set();

    dispose(this.toDispose);
  }

  getLabelString(): string {
    const label = this.item.getLabel();

    if (typeof label === 'string') {
      return label;
    }

    return label.label;
  }

  shouldRenderChildren(): boolean {
    if (!this.item.shouldRenderChildren) {
      return true;
    }

    return this.item.shouldRenderChildren();
  }

  readonly layoutEffect = (): () => void => {
    this.rendered = true;

    if (this.pendingScrollIntoView) {
      this.pendingScrollIntoView = false;

      requestAnimationFrame(() => this.doScrollIntoView());
    }

    return () => {
      this.rendered = false;
    };
  };

  setSelected(selected: boolean, scrollIntoView = true) {
    if (!selected) {
      return this.selectionFlag.close();
    }

    this.selectionFlag.open();

    if (scrollIntoView) {
      this.scrollIntoView();
    }
  }

  scrollIntoView() {
    if (this.rendered) {
      this.doScrollIntoView();
    } else {
      this.pendingScrollIntoView = true;
    }
  }

  private doScrollIntoView() {
    ((this.ref.current as any)?.scrollIntoViewIfNeeded as HTMLDivElement['scrollIntoView'])({
      block: 'center',
    });
  }
}
