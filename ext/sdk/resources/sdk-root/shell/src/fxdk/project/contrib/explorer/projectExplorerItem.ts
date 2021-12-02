import { Disposer, IDisposableObject } from "fxdk/base/disposable";
import { ContextMenuItemsCollection } from "fxdk/ui/controls/ContextMenu/ContextMenu";
import { DropTargetMonitor } from "react-dnd";
import { IDynamicShellCommand } from "shell-api/commands";
import { OpenFlag } from "store/generic/OpenFlag";

export interface IProjectExplorerItem extends IDisposableObject {
  readonly id: string;

  acceptParentContext?(parentContext: IProjectExplorer.ItemContext): void;
  getContext?(): IProjectExplorer.ItemContext;

  forceNoChildren?(): boolean;

  computeMenuItems?(disposer: Disposer): ContextMenuItemsCollection | undefined;
  computeRenamer?(disposer: Disposer): Readonly<IProjectExplorer.ItemRenamer> | undefined;
  computeDragAndDrop?(disposer: Disposer): Readonly<IProjectExplorer.ItemDragAndDrop> | undefined;

  /**
   * Specify item's state that is that little dot you see over item's icon in project explorer
   */
  getState?(): IProjectExplorer.ItemState | undefined;

  getChildren?(): IProjectExplorerItem[];
  shouldRenderChildren?(): boolean;

  /**
   * Return default item icon that is not depending on anything but expanded param
   */
  getDefaultIcon(expanded: boolean): React.ReactNode;

  getIcon(): IProjectExplorer.ItemIcon;
  getLabel(): IProjectExplorer.ItemLabel;

  getTitle?(): React.ReactNode;
  getStatus?(): React.ReactNode;

  renderCreator?(): React.ReactNode;
  renderInWrapper?(): React.ReactNode;

  handleClick?(): void;
  handleDoubleClick?(): void;
}

export namespace IProjectExplorer {
  export type ItemContextEntry =
    | null
    | boolean
    | undefined | void
    | string
    | symbol
    | number
    | ReadonlyArray<ItemContextEntry>
    | ReadonlyMap<any, ItemContextEntry>
    | ReadonlySet<ItemContextEntry>
    | Function
    | ItemContext;

  export type ItemContext = Readonly<{
    [key: string | symbol]: ItemContextEntry;
  }>;

  export interface ItemRenamer {
    command: IDynamicShellCommand,
    state: OpenFlag,

    validate(name: string): boolean,
    handleRename(name: string): void,
  }

  export type ItemLabel = string | { className: string, label: string };

  export type ItemIcon = React.ReactNode | { className: string, icon: React.ReactNode };

  export interface ItemDragItem {
    type: string,
  }

  export interface ItemDrag {
    getItem(): ItemDragItem;
    canDrag?(): boolean;
  }

  export interface ItemDrop {
    acceptTypes: string[];
    handleDrop(item: unknown, monitor: DropTargetMonitor): void;
    canDrop?(item: unknown, monitor: DropTargetMonitor): boolean;
  }

  export interface ItemDragAndDrop {
    drag?: ItemDrag;
    drop?: ItemDrop;
  }

  export interface ItemRename {
    validate(name: string): boolean;
    handleRename(name: string): void;
  }

  export interface ItemState {
    enabled: boolean,
    running?: boolean,
  }
}
