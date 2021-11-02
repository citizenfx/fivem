import { ContextMenuItemsCollection, ContextMenuItemsGroup, SimpleContextMenuItem } from "fxdk/ui/controls/ContextMenu/ContextMenu";
import { IBaseRegistryItem, Registry } from "fxdk/base/registry";
import { IBaseViewRegistryItem, viewRegistryVisibilityFilter } from "fxdk/base/viewRegistry";

const Ungrouped = Symbol('Ungrouped');
type IUngrouped = typeof Ungrouped;

export interface IToolbarMainMenuAddition extends IBaseRegistryItem {
  order?: number,
  group?: string,
  item: SimpleContextMenuItem,
}

export interface IToolbarViewParticipant extends IBaseViewRegistryItem {
}

export interface IToolbarTitleViewParticipant extends IBaseViewRegistryItem {
}

export const ToolbarParticipants = new class ToolbarParticipants {
  private menuRegistry = new Registry<IToolbarMainMenuAddition>('toolbar-menu-participants', true);
  private viewRegistry = new Registry<IToolbarViewParticipant>('toolbar-view-participants', true);
  private titleViewRegistry = new Registry<IToolbarTitleViewParticipant>('toolbar-title-view-participants', true);

  private mainMenuAdditions: ContextMenuItemsCollection = [];

  registerMenuItem(participant: IToolbarMainMenuAddition) {
    this.menuRegistry.register(participant);

    this.recalculateMainMenuAdditions();
  }

  registerView(participant: IToolbarViewParticipant) {
    this.viewRegistry.register(participant);
  }

  registerTitleView(participant: IToolbarTitleViewParticipant) {
    this.titleViewRegistry.register(participant);
  }

  getMainMenuAdditions() {
    return this.mainMenuAdditions;
  }

  getAllVisibleViews() {
    return this.viewRegistry.getAll().filter(viewRegistryVisibilityFilter);
  }

  getAllVisibleTitleViews() {
    return this.titleViewRegistry.getAll().filter(viewRegistryVisibilityFilter);
  }

  private recalculateMainMenuAdditions() {
    const sortedItems = this.menuRegistry.getAll().slice().sort(menuItemsSorter);

    const groupedItems: Record<string | IUngrouped, ContextMenuItemsGroup> = {
      [Ungrouped]: {
        items: [],
      },
    };

    for (const item of sortedItems) {
      const group = item.group || Ungrouped;

      groupedItems[group] ??= { items: [] };
      groupedItems[group].items.push(item.item);
    }

    this.mainMenuAdditions = Object.values(groupedItems);
  }
}();

function menuItemsSorter(a: IToolbarMainMenuAddition, b: IToolbarMainMenuAddition): number {
  return (a.order || Number.MAX_SAFE_INTEGER) - (b.order || Number.MAX_SAFE_INTEGER);
}
