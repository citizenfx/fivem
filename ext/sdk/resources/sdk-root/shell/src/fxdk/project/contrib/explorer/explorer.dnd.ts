import React from 'react';
import { returnTrue } from 'fxdk/base/functional';
import { DragSourceMonitor, DropTargetMonitor, useDrag, useDrop } from 'react-dnd';
import { RuntimeItem } from "./explorer.runtime";
import { NativeTypes } from 'react-dnd-html5-backend';
import { copyItemIntoEntryFromDrop } from './basics';
import sItem from './item.module.scss';

export const projectExplorerItemType = {
  FILE: 'projectExplorer:file',
  FOLDER: 'projectExplorer:folder',
  ASSET: 'projectExplorer:asset',
};

export interface NativeDropItem {
  files: (File & { path: string })[],
}

export interface FxDKDropItem {
  entryPath?: string,
  type: string | Symbol,
}
export type EntryMoveItem =
  | NativeDropItem
  | FxDKDropItem;


export function useItemDroppable(item: RuntimeItem, classNames: Record<string, unknown>): React.Ref<any> {
  // Since dnd config is readonly and will not change over lifetime of RuntimeItem it is safe to have conditional hooks here
  if (!item.dragAndDrop?.drop) {
    return React.useRef();
  }

  const { canDrop = returnTrue, acceptTypes, handleDrop } = item.dragAndDrop.drop;

  const [{ isDropping }, dropRef] = useDrop({
    accept: acceptTypes,
    canDrop,
    drop(dropItem: unknown, monitor: DropTargetMonitor) {
      if (monitor.didDrop()) {
        return;
      }

      handleDrop(dropItem, monitor);
    },
    collect: dropCollector,
  });

  classNames[sItem.dropping] = isDropping;

  return dropRef;
}

export function useItemDraggable(item: RuntimeItem, classNames: Record<string, unknown>): React.Ref<any> {
  // Since dnd config is readonly and will not change over lifetime of RuntimeItem it is safe to have conditional hooks here
  if (!item.dragAndDrop?.drag) {
    return React.useRef();
  }

  const { getItem, canDrag = returnTrue } = item.dragAndDrop.drag;

  const [{ isDragging }, dragRef] = useDrag({
    item: getItem(),
    canDrag,
    collect: dragCollector,
  });

  classNames[sItem.dragging] = isDragging;

  return dragRef;
}

export function useRootDrop(entryPath: string): { isDropping: boolean, dropRef: React.Ref<any> } {
  const [{ isDropping }, dropRef] = useDrop({
    accept: [
      projectExplorerItemType.ASSET,
      projectExplorerItemType.FOLDER,
      projectExplorerItemType.FILE,
      NativeTypes.FILE,
    ],
    collect: dropCollector,
    drop: (item, monitor) => {
      if (monitor.didDrop()) {
        return;
      }

      copyItemIntoEntryFromDrop(entryPath, item);
    },
  });

  return { isDropping, dropRef };
}

function dropCollector(monitor: DropTargetMonitor) {
  return {
    isDropping: monitor.isOver({ shallow: true }) && monitor.canDrop(),
  };
}

function dragCollector(monitor: DragSourceMonitor) {
  return {
    isDragging: monitor.isDragging(),
  };
}
