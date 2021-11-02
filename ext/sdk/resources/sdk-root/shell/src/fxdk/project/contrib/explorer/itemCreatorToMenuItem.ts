import { returnFalse, returnTrue } from "fxdk/base/functional";
import { IMenuItem } from "fxdk/base/menu";
import { IShellCommandId } from "shell-api/commands";
import { IExplorerItemCreator } from "./explorer.itemCreate";
import { ProjectExplorerItemMenuGroups } from "./explorer.itemMenu";
import { IProjectExplorer } from "./projectExplorerItem";

export function convertItemCreatorToMenuItem(itemCreator: IExplorerItemCreator, commandId: IShellCommandId, ctx?: IProjectExplorer.ItemContext): IMenuItem {
  const { id, icon, label, order, visible, disabled } = itemCreator;

  return {
    id: `createItem(${id})`,
    icon,
    order,
    label: `New ${label}`,
    commandId,
    group: ProjectExplorerItemMenuGroups.CREATE,
    visible: wrapWithContext(visible, returnTrue, ctx),
    disabled: wrapWithContext(disabled, returnFalse, ctx),
  };
}

function wrapWithContext(thing: undefined | ((ctx?: IProjectExplorer.ItemContext) => boolean), defaulter: () => boolean, ctx?: IProjectExplorer.ItemContext): () => boolean {
  if (!thing) {
    return defaulter;
  }

  return () => thing(ctx);
}
