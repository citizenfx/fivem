import { IBaseRegistryItem } from "fxdk/base/registry";
import { IShellCommandId } from "shell-api/commands";
import { ProjectExplorerFileSystemItemTypes } from "./explorer.fileSystemItem";
import { IProjectExplorer } from "./projectExplorerItem";

type NoType<T extends { type: string }> = Omit<T, 'type'>;

interface BaseExplorerItemCreator extends IBaseRegistryItem {
  icon: React.ReactNode | ((name?: string) => React.ReactNode),
  label: string;

  order?: number;
  placeholder?: string;

  disabled?(context?: IProjectExplorer.ItemContext): boolean;
  visible?(context?: IProjectExplorer.ItemContext): boolean;
}

export interface IInlineExplorerItemCreator extends BaseExplorerItemCreator {
  readonly type: 'inline';

  nameValidator?: ProjectExplorerFileSystemItemTypes.RenameValidator,
  createCommandId: IShellCommandId;
}
export function inlineExplorerItemCreator(creator: NoType<IInlineExplorerItemCreator>): IInlineExplorerItemCreator {
  return {
    type: 'inline',
    ...creator,
  };
}

export interface ICustomExplorerItemCreator extends BaseExplorerItemCreator {
  readonly type: 'custom';

  openCommandId: IShellCommandId;
}
export function customExplorerItemCreator(creator: NoType<ICustomExplorerItemCreator>): ICustomExplorerItemCreator {
  return {
    type: 'custom',
    ...creator,
  };
}

export type IExplorerItemCreator =
  | IInlineExplorerItemCreator
  | ICustomExplorerItemCreator;

export function getCreatorIcon(creator: IExplorerItemCreator,name?: string): React.ReactNode {
  if (typeof creator.icon === 'function') {
    return creator.icon(name);
  }

  return creator.icon;
}
