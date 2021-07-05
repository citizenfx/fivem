import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { DropTargetMonitor, useDrop } from 'react-dnd';
import { mapExplorerDNDTypes } from '../MapExplorer.constants';
import { RiMapPinAddFill } from 'react-icons/ri';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { MapExplorerGroup } from '../MapExplorerGroup/MapExplorerGroup';
import { MapExplorerAdditionsItem } from './MapExplorerAdditionsItem';
import { ContextMenu, ContextMenuItemsCollection } from 'components/controls/ContextMenu/ContextMenu';
import { newDirectoryIcon } from 'constants/icons';
import { useOpenFlag } from 'utils/hooks';
import { WORLD_EDITOR_MAP_NO_GROUP } from 'backend/world-editor/world-editor-constants';
import { InPlaceInput } from 'components/controls/InPlaceInput/InPlaceInput';
import s from './MapExplorerAdditions.module.scss';

export const MapExplorerAdditions = observer(function MapExplorerAdditions() {
  const [groupCreatorOpen, openGroupCreator, closeGroupCreator] = useOpenFlag(false);

  const handleGroupNameChange = React.useCallback((newGroupName: string) => {
    closeGroupCreator();

    if (newGroupName) {
      WorldEditorState.map.createAdditionGroup(newGroupName);
    }
  }, [closeGroupCreator]);

  React.useEffect(() => {
    if (groupCreatorOpen) {
      return WorldEditorState.overrideInput();
    }
  }, [groupCreatorOpen]);

  const [{ isDropping }, dropRef] = useDrop({
    accept: mapExplorerDNDTypes.ADDITION,
    drop(item: unknown | { id: string, type: string }, monitor: DropTargetMonitor) {
      if (monitor.didDrop()) {
        return;
      }

      if (item['id']) {
        WorldEditorState.map.setAdditionGroup(item['id'], WORLD_EDITOR_MAP_NO_GROUP);
      }
    },
    collect: (monitor) => ({
      isDropping: monitor.isOver({ shallow: true }) && monitor.canDrop(),
    }),
  });

  const rootClassName = classnames(s.root, {
    [s.dropping]: isDropping,
  });

  const contextMenu = React.useMemo(() => [
    {
      id: 'create-group',
      text: 'Create group',
      icon: newDirectoryIcon,
      onClick: openGroupCreator,
    },
  ] as ContextMenuItemsCollection, [openGroupCreator]);

  return (
    <ContextMenu
      ref={dropRef}
      className={rootClassName}
      activeClassName={s.active}
      items={contextMenu}
    >
      <header>
        <div className={s.icon}>
          <RiMapPinAddFill />
        </div>
        <div className={s.name}>
          Map additions
        </div>
      </header>

      <div className={s.children}>
        {groupCreatorOpen && (
          <div className={s.creator}>
            {newDirectoryIcon}
            <InPlaceInput
              value=""
              onChange={handleGroupNameChange}
              placeholder="New group name"
            />
          </div>
        )}

        {Object.keys(WorldEditorState.map.additionGroups).map((grp) => {
          return (
            <MapExplorerGroup
              key={grp}
              grp={grp}
              items={WorldEditorState.map.getGroupAdditions(grp)}
            />
          );
        })}

        {Object.entries(WorldEditorState.map.additionsUngrouped).map(([id, obj]) => (
          <MapExplorerAdditionsItem
            id={id}
            key={id}
            item={obj}
            onLabelChange={(label: string) => label.trim() && WorldEditorState.map?.setAdditionLabel(id, label.trim())}
            onClick={() => WorldEditorState.setCam(obj.cam)}
            onDelete={() => WorldEditorState.map.deleteAddition(id)}
          />
        ))}
      </div>
    </ContextMenu>
  );
});
