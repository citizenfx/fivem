import React from 'react';
import classnames from 'classnames';
import { FiBox } from 'react-icons/fi';
import { WETool, WorldEditorToolbarState } from '../WorldEditorToolbarState';
import { BaseTool } from '../BaseTool/BaseTool';
import { useOpenFlag } from 'utils/hooks';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { DropTargetMonitor, useDrop } from 'react-dnd';
import { ADDITION_DND_TYPES } from './AdditionsTool.constants';
import { WORLD_EDITOR_MAP_NO_GROUP } from 'backend/world-editor/world-editor-constants';
import s from './AdditionsTool.module.scss';
import { newDirectoryIcon } from 'constants/icons';
import { ContextMenu, ContextMenuItemsCollection } from 'components/controls/ContextMenu/ContextMenu';
import { InPlaceInput } from 'components/controls/InPlaceInput/InPlaceInput';
import { Addition } from './Addition';
import { AdditionsGroup } from './AdditionsGroup/AdditionsGroup';
import { AdditionsNewGroup } from './AdditionsNewGroup/AdditionsNewGroup';
import { observer } from 'mobx-react-lite';

export const AdditionsTool = observer(function AdditionsTool() {
  const [groupCreatorOpen, openGroupCreator, closeGroupCreator] = useOpenFlag(false);

  const handleGroupNameChange = React.useCallback((newGroupName: string) => {
    closeGroupCreator();

    if (newGroupName) {
      WEState.map.createAdditionGroup(newGroupName);
    }
  }, [closeGroupCreator]);

  React.useEffect(() => {
    if (groupCreatorOpen) {
      return WEState.overrideInput();
    }
  }, [groupCreatorOpen]);

  const [{ isDropping }, dropRef] = useDrop({
    accept: ADDITION_DND_TYPES.ADDITION,
    drop(item: unknown | { id: string, type: string }, monitor: DropTargetMonitor) {
      if (monitor.didDrop()) {
        return;
      }

      if (item['id']) {
        WEState.map.setAdditionGroup(item['id'], WORLD_EDITOR_MAP_NO_GROUP);
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
    <BaseTool
      renderAlways
      tool={WETool.Additions}
      icon={<FiBox />}
      label="Map additions"
    >
      <ContextMenu
        ref={dropRef}
        className={rootClassName}
        activeClassName={s.active}
        items={contextMenu}
      >
        <AdditionsNewGroup />

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

        {Object.keys(WEState.map.additionGroups).map((grp) => {
          return (
            <AdditionsGroup
              key={grp}
              grp={grp}
              items={WEState.map.getGroupAdditions(grp)}
            />
          );
        })}

        {Object.entries(WEState.map.additionsUngrouped).map(([id, obj]) => (
          <Addition
            id={id}
            key={id}
            item={obj}
            onLabelChange={(label: string) => label.trim() && WEState.map?.setAdditionLabel(id, label.trim())}
            onClick={() => WEState.setCam(obj.cam)}
            onDelete={() => WEState.map.deleteAddition(id)}
          />
        ))}
      </ContextMenu>
    </BaseTool>
  );
});
