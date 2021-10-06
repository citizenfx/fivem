/**
 * WIP Project Explorer tree rework, don't use yet
 */

import { ContextMenuItemsCollection } from "fxdk/ui/controls/ContextMenu/ContextMenu";
import { ItemStateProps } from "fxdk/project/browser/ProjectExplorer/ItemState";
import { openInExplorerIcon } from "constants/icons";
import { FilesystemEntry } from "shared/api.types";
import { ProjectObject } from "store/ProjectState";
import { openInExplorer, openInExplorerAndSelect } from "utils/natives";

export interface IAssetViewOptions {
  icon?: React.ReactNode,
  contextMenuItems?: ContextMenuItemsCollection,
  noOpenInExplorer?: boolean,
}

export abstract class AssetView {
  constructor(
    protected readonly entry: Readonly<FilesystemEntry>,
    protected readonly project: ProjectObject,
    private readonly options?: Readonly<IAssetViewOptions>,
  ) {
  }

  /**
   * @api
   */
  getContextMenuItems(): ContextMenuItemsCollection {
    return this.options?.contextMenuItems || [];
  }

  /**
   * @api
   *
   * Override to customize icon rendering
   */
  getIcon(): React.ReactNode {
    return this.options?.icon || null;
  }

  /**
   * @api
   *
   * Override to customize label rendering
   */
  getLabel(): React.ReactNode {
    return this.entry.name;
  }

  /**
   * @api
   *
   * Override to customize state props
   */
  getState(): ItemStateProps | null {
    return null;
  }

  /**
   * @api
   *
   * Override to customize status rendering
   */
  getStatus(): React.ReactNode {
    return null;
  }






  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // BELOW ARE INTRINSICS, SEE METHODS ABOVE TO CUSTOMIZE

  get contextMenuItems(): ContextMenuItemsCollection {
    if (this.options?.noOpenInExplorer) {
      return this.getContextMenuItems();
    }

    return [
      ...this.getContextMenuItems(),
      {
        id: 'open-in-explorer',
        icon: openInExplorerIcon,
        text: 'Open in Explorer',
        onClick: () => {
          if (this.entry.isDirectory) {
            return openInExplorer(this.entry.path);
          } else {
            return openInExplorerAndSelect(this.entry.path);
          }
        },
      },
    ];
  }
}
