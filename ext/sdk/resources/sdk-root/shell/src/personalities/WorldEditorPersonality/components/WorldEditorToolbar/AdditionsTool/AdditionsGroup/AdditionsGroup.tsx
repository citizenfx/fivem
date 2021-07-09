import React from 'react';
import classnames from 'classnames';
import { WEMapAddition } from 'backend/world-editor/world-editor-types';
import { observer } from 'mobx-react-lite';
import { useOpenFlag } from 'utils/hooks';
import { closedDirectoryIcon, deleteIcon, openDirectoryIcon, renameIcon } from 'constants/icons';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { ContextMenu, ContextMenuItemsCollection } from 'components/controls/ContextMenu/ContextMenu';
import { DropTargetMonitor, useDrop } from 'react-dnd';
import { InPlaceInput } from 'components/controls/InPlaceInput/InPlaceInput';
import s from './AdditionsGroup.module.scss';
import { ADDITION_DND_TYPES } from '../AdditionsTool.constants';
import { Addition } from '../Addition';
import { AdditionsGroupDeleter } from './AdditionsGroupDeleter/AdditionsGroupDeleter';

export interface AdditionsGroupProps {
  grp: string,
  items: Record<string, WEMapAddition>,
}

export const AdditionsGroup = observer(function AdditionsGroup(props: AdditionsGroupProps) {
  const { grp, items } = props;

  const group = WEState.map.additionGroups[grp];

  const [collapsed, _expand, _collapse, toggleCollapsed] = useOpenFlag(false);

  const [{ isDropping }, dropRef] = useDrop({
    accept: ADDITION_DND_TYPES.ADDITION,
    drop(item: unknown | { id: string, type: string }, monitor: DropTargetMonitor) {
      if (monitor.didDrop()) {
        return;
      }

      if (item['id']) {
        WEState.map.setAdditionGroup(item['id'], grp);
      }
    },
    collect: (monitor) =>({
      isDropping: monitor.isOver({ shallow: true }) && monitor.canDrop(),
    }),
  });

  const rootClassName = classnames(s.root, {
    [s.collapsed]: collapsed,
    [s.dropping]: isDropping,
  });

  const [groupRenamerOpen, openGroupRenamer, closeGroupRenamer] = useOpenFlag(true);
  const [groupDeleterOpen, openGroupDeleter, closeGroupDeleter] = useOpenFlag(false);

  React.useEffect(() => {
    if (groupRenamerOpen) {
      return WEState.overrideInput();
    }
  }, [groupRenamerOpen]);

  const handleGroupLabelChange = React.useCallback((newLabel: string) => {
    closeGroupRenamer();

    WEState.map.setAdditionGroupLabel(grp, newLabel);
  }, [grp, closeGroupRenamer]);

  const handleGroupDelete = React.useCallback(() => {
    if (Object.keys(items).length) {
      openGroupDeleter();
    } else {
      WEState.map.deleteAdditionGroup(grp, false);
    }
  }, [grp, items]);

  const contextMenu = React.useMemo(() => [
    {
      id: 'rename',
      text: 'Rename group',
      icon: renameIcon,
      onClick: openGroupRenamer,
    },
    {
      id: 'delete',
      text: 'Delete group',
      icon: deleteIcon,
      onClick: handleGroupDelete,
    },
  ] as ContextMenuItemsCollection, [openGroupRenamer, handleGroupDelete]);

  const headerClassName = classnames(s.header, {
    [s.editing]: groupRenamerOpen,
  });

  return (
    <div className={rootClassName} ref={dropRef}>
      {
        groupRenamerOpen
          ? (
            <div className={headerClassName}>
              <div className={s.icon}>
                {collapsed ? closedDirectoryIcon : openDirectoryIcon}
              </div>
              <InPlaceInput
                value={group?.label}
                onChange={handleGroupLabelChange}
                placeholder="Group label"
              />
            </div>
          )
          : (
            <ContextMenu
              className={s.header}
              activeClassName={s.active}
              onClick={toggleCollapsed}
              items={contextMenu}
            >
              <div className={s.icon}>
                {collapsed ? closedDirectoryIcon : openDirectoryIcon}
              </div>
              <div className={s.name}>
                {group?.label}
              </div>
            </ContextMenu>
          )
      }

      <div className={s.children}>
        {Object.entries(items).map(([id, obj]) => (
          <Addition
            id={id}
            key={id}
            item={obj}
            onLabelChange={(label: string) => label.trim() && WEState.map?.setAdditionLabel(id, label.trim())}
            onClick={() => WEState.setCam(obj.cam)}
            onDelete={() => WEState.map.deleteAddition(id)}
          />
        ))}
      </div>

      {groupDeleterOpen && (
        <AdditionsGroupDeleter
          grp={grp}
          onClose={closeGroupDeleter}
        />
      )}
    </div>
  );
});
