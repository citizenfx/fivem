import React from 'react';
import classnames from 'classnames';
import mergeRefs from 'utils/mergeRefs';
import { observer } from 'mobx-react-lite';
import { Title } from 'fxdk/ui/controls/Title/Title';
import { InPlaceInput } from 'fxdk/ui/controls/InPlaceInput/InPlaceInput';
import { ContextMenu, ContextMenuProps } from 'fxdk/ui/controls/ContextMenu/ContextMenu';
import { ItemState as BaseItemState } from './ui/ItemState';
import { InlineItemCreator } from './ui/InlineItemCreator/InlineItemCreator';
import { entriesSorter } from './explorer.sorters';
import { RuntimeItem, ExplorerRuntime } from './explorer.runtime';
import { useItemDraggable, useItemDroppable, useRootDrop } from './explorer.dnd';
import { fsEntriesFilter, rootFsEntriesFilter } from './explorer.filters';
import { EmptyExplorerView } from './EmptyExplorerView';
import { joinPath, renderExplorerItemIcon } from './explorer.utils';
import { Project } from 'fxdk/project/browser/state/project';
import sItem from './item.module.scss';
import s from './ProjectExplorer.module.scss';

interface WithItem {
  runtimeItem: RuntimeItem,
}

interface WithPath {
  path: string[],
  basePath: string,
}

interface WithChildren {
  children: React.ReactNode,
}

const ItemMain = observer(function ProjectExplorerItemNode({ runtimeItem, children }: WithItem & WithChildren) {
  const rootClassNames: Record<string, unknown> = {
    [sItem.selected]: runtimeItem.selectionFlag.isOpen,
  };
  const rootRef = useItemDraggable(runtimeItem, rootClassNames);

  const contentMenuProps: Partial<ContextMenuProps> & Required<Pick<ContextMenuProps, 'items'>> = {
    items: runtimeItem.menuItems,
    className: classnames(sItem.label, rootClassNames),
    activeClassName: sItem.active,
    onClick: runtimeItem.item.handleClick,
    onDoubleClick: runtimeItem.item.handleDoubleClick,
  };

  return (
    <Title title={runtimeItem.item.getTitle?.()}>
      {(ref) => (
        <ContextMenu
          ref={mergeRefs(ref, rootRef)}
          {...contentMenuProps}
        >
          {children}
        </ContextMenu>
      )}
    </Title>
  );
});

const ItemChildren = observer(function ProjectExplorerItemChildren({ runtimeItem, path, basePath }: WithItem & WithPath) {
  if (!runtimeItem.shouldRenderChildren()) {
    return null;
  }

  const nodes: React.ReactNode[] = [];

  if (runtimeItem.item.getChildren) {
    for (const childItem of runtimeItem.item.getChildren()) {
      const childRuntimeItem = ExplorerRuntime.getOrCreateItem(childItem, runtimeItem);

      nodes.push(
        <ProjectExplorerItem key={childRuntimeItem.id} runtimeItem={childRuntimeItem} path={path} basePath={basePath} />
      );
    }
  }

  const fsEntries = Project.fileSystem.getChildren(path);
  if (fsEntries) {
    for (const entry of fsEntries.filter(fsEntriesFilter).sort(entriesSorter)) {
      const childPath = [...path, entry.name];
      const childFileSystemRuntimeItem = ExplorerRuntime.getOrCreateFileSystemItem(entry, joinPath(basePath, ...childPath), runtimeItem);

      nodes.push(
        <ProjectExplorerItem key={childFileSystemRuntimeItem.id} runtimeItem={childFileSystemRuntimeItem} path={childPath} basePath={basePath} />
      );
    }
  }

  return (
    <div className={sItem.children}>
      {nodes}
    </div>
  );
});

const ItemState = observer(function ProjectExplorerItemState({ runtimeItem }: WithItem) {
  const state = runtimeItem.item.getState?.();
  if (!state) {
    return null;
  }

  return (
    <BaseItemState {...state} />
  );
});

const ItemIcon = observer(function ProjectExplorerIcon({ runtimeItem }: WithItem) {
  return renderExplorerItemIcon(runtimeItem.item.getIcon(), sItem.icon) as any;
});

const ItemLabel = observer(function ProjectExplorerItemLabel({ runtimeItem }: WithItem) {
  const label = runtimeItem.item.getLabel();

  if (typeof label === 'object') {
    return (
      <div className={classnames(sItem.title, label.className)}>
        {label.label}
      </div>
    );
  }

  return (
    <div className={sItem.title}>
      {label}
    </div>
  );
});

const ItemStatus = observer(function ItemStatus({ runtimeItem }: WithItem) {
  if (!runtimeItem.item.getStatus) {
    return null;
  }

  return (
    <div className={sItem.status}>
      {runtimeItem.item.getStatus()}
    </div>
  );
});

const ItemMainRenaming = observer(function ItemMainRenaming({ runtimeItem, children }: WithItem & WithChildren) {
  const rename = runtimeItem.renamer!;
  const label = runtimeItem.getLabelString();

  const handleRename = React.useCallback((newLabel: string) => {
    if (label === newLabel) {
      rename.state.close();
      return;
    }

    rename.handleRename(newLabel);
    rename.state.close();
  }, [rename, label]);

  return (
    <div className={classnames(sItem.label, sItem.renaming)}>
      {children}

      <InPlaceInput
        value={label}
        validate={rename.validate}
        onChange={handleRename}
        className={sItem.renamer}
      />
    </div>
  );
});

const ProjectExplorerItem = observer(function ProjectExplorerItem({ runtimeItem, path, basePath }: WithItem & WithPath) {
  React.useLayoutEffect(runtimeItem.layoutEffect, []);

  const rootExtraClassNames: Record<string, unknown> = {};

  const dropRef = useItemDroppable(runtimeItem, rootExtraClassNames);
  const rootClassName = classnames(sItem.root, rootExtraClassNames);

  const renderInWrapperNode = runtimeItem.item.renderInWrapper?.() || null;
  const creatorNode = runtimeItem.item.renderCreator?.() || null;

  return (
    <div ref={mergeRefs(runtimeItem.ref, dropRef)} className={rootClassName}>
      <div className={sItem['label-container']}>
        {getItemMainNode(runtimeItem)}

        {!!runtimeItem.hasGetStatus && (
          <ItemStatus runtimeItem={runtimeItem} />
        )}
      </div>

      {creatorNode}

      {renderInWrapperNode}

      {!runtimeItem.noChildren && (
        <ItemChildren runtimeItem={runtimeItem} path={path} basePath={basePath} />
      )}
    </div>
  );
});

export const ExplorerView = observer(function ExplorerView() {
  let creatorNode: React.ReactNode = null;
  if (ExplorerRuntime.rootInlineItemCreatorState.isOpen && ExplorerRuntime.rootInlineItemCreator) {
    creatorNode = (
      <InlineItemCreator
        close={ExplorerRuntime.rootInlineItemCreatorState.close}
        creator={ExplorerRuntime.rootInlineItemCreator}
        basePath={Project.path}
      />
    );
  }

  // Allow paint earlier
  const [renderAll, setRenderAll] = React.useState(false);
  React.useEffect(() => {
    requestAnimationFrame(() => setRenderAll(true));
  }, []);

  let rootNodes: React.ReactNode[] | null = null;
  if (renderAll) {
    rootNodes = Project.fileSystem.getRootChildren().filter(rootFsEntriesFilter).sort(entriesSorter).map((entry) => {
      const item = ExplorerRuntime.getOrCreateFileSystemItem(entry, joinPath(Project.path, entry.name));

      return (
        <ProjectExplorerItem key={item.id} runtimeItem={item} path={[entry.name]} basePath={Project.path} />
      );
    });
  }

  const { isDropping, dropRef } = useRootDrop(Project.path);

  const rootClassName = classnames(s.root, {
    [s.dropping]: isDropping,
  });

  let mainNode: React.ReactNode = null;
  if (rootNodes === null) {
    mainNode = null;
  } else if (rootNodes.length) {
    mainNode = rootNodes;
  } else {
    mainNode = (
      <EmptyExplorerView />
    );
  }

  return (
    <ContextMenu
      ref={dropRef}
      items={ExplorerRuntime.getRootMenuItems()}
      className={rootClassName}
      elementProps={{ 'data-intro-id': 'project-explorer' }}
    >
      {creatorNode}
      {mainNode}
    </ContextMenu>
  );
});

function getItemMainNode(item: RuntimeItem): React.ReactNode {
  const commons = (
    <>
      <ItemState runtimeItem={item} />
      <ItemIcon runtimeItem={item} />
    </>
  );

  if (item.renamer?.state.isOpen) {
    return (
      <ItemMainRenaming runtimeItem={item}>
        {commons}
      </ItemMainRenaming>
    );
  }

  return (
    <ItemMain runtimeItem={item}>
      {commons}
      <ItemLabel runtimeItem={item} />
    </ItemMain>
  );
}
