import { ContextMenuItem, ContextMenuItemsCollection, ContextMenuItemsGroup } from "fxdk/ui/controls/ContextMenu/ContextMenu";
import { IShellCommandId, ShellCommands } from "shell-api/commands";
import { alphabeticalOrderCollator } from "./collators";

const Ungrouped = '99999_$ungrouped';

type ReactNodeOrFactory =
  | React.ReactNode
  | (() => React.ReactNode);

type CommandIdOrFactory =
  | IShellCommandId
  | (() => IShellCommandId);

export interface IMenuItem {
  id: string;
  label: ReactNodeOrFactory;
  icon?: ReactNodeOrFactory;
  group?: string,
  order?: number,
  visible?(): boolean,
  disabled?(): boolean,
  commandId: CommandIdOrFactory;
  commandArgs?(): unknown[];
}

export function convertMenuItemsToContextMenuItems(items: IMenuItem[] | ReadonlyArray<IMenuItem>): ContextMenuItemsCollection {
  if (!items.length) {
    return [];
  }

  const sortedItems = items.slice().sort(menuItemsSorter);

  const groupedItems: Record<string, ContextMenuItemsGroup> = {
    [Ungrouped]: {
      items: [],
    },
  };

  for (const item of sortedItems) {
    const group = item.group || Ungrouped;

    groupedItems[group] ??= { items: [] };
    groupedItems[group].items.push({
      id: item.id,
      text: item.label,
      icon: item.icon,
      visible: item.visible,
      disabled: item.disabled,
      onClick() {
        const commandId = getCommandId(item.commandId);

        if (item.commandArgs) {
          return ShellCommands.invoke(commandId, ...item.commandArgs());
        }

        ShellCommands.invoke(commandId);
      },
    } as ContextMenuItem);
  }

  return Object.keys(groupedItems).sort(menuGroupSorter).reduce((acc, group) => {
    if (groupedItems[group].items.length) {
      acc.push(groupedItems[group]);
    }

    return acc;
  }, [] as ContextMenuItemsGroup[]);
}

function getCommandId(commandId: CommandIdOrFactory): IShellCommandId {
  if (typeof commandId === 'function') {
    return commandId();
  }

  return commandId;
}

function menuGroupSorter(a: string, b: string): number {
  return alphabeticalOrderCollator.compare(a, b);
}

function menuItemsSorter(a: IMenuItem, b: IMenuItem): number {
  return (a.order || Number.MAX_SAFE_INTEGER) - (b.order || Number.MAX_SAFE_INTEGER);
}
