import React from 'react';
import { IBaseRegistryItem } from "./registry";

export interface IBaseViewRegistryItem extends IBaseRegistryItem {
  isVisible?(): boolean;

  render(): React.ReactNode;
}

export function viewRegistryVisibilityFilter(item: IBaseViewRegistryItem): boolean {
  if (item.isVisible) {
    return item.isVisible();
  }

  return true;
}

export function renderViewRegistryItems(items: IBaseViewRegistryItem[]): React.ReactNode {
  return items.map((item) => (
    <React.Fragment key={item.id}>
      {item.render()}
    </React.Fragment>
  ));
}
