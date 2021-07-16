import React from 'react';
import classnames from 'classnames';
import { newDirectoryIcon, openDirectoryIcon } from 'constants/icons';
import { DropTargetMonitor, useDrop } from 'react-dnd';
import { ADDITION_DND_TYPES } from '../AdditionsTool.constants';
import { WORLD_EDITOR_MAP_NO_GROUP } from 'backend/world-editor/world-editor-constants';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import s from './AdditionsNewGroup.module.scss';

export const AdditionsNewGroup = React.memo(function AdditionsNewGroup() {
  const [{ isDroppingNewGroup, isAccepting }, dropRef] = useDrop({
    accept: ADDITION_DND_TYPES.ADDITION,
    drop(item: unknown | { id: string, type: string }, monitor: DropTargetMonitor) {
      if (monitor.didDrop()) {
        return;
      }

      if (item['id']) {
        const grp = WEState.map.createAdditionGroup(`New group (${WEState.map.additions[item['id']].label})`);

        WEState.map.setAdditionGroup(item['id'], grp);
      }
    },
    collect: (monitor) => ({
      isDroppingNewGroup: monitor.isOver({ shallow: true }) && monitor.canDrop(),
      isAccepting: monitor.canDrop(),
    }),
  });

  const handleCreateGroup = React.useCallback(() => {
    WEState.map.createAdditionGroup('New group');
  }, []);

  const rootClassName = classnames(s.root, {
    [s.accepting]: isAccepting,
  });

  const newGroupClassName = classnames({
    [s.dropping]: isDroppingNewGroup,
  });

  return (
    <div className={rootClassName}>
      <button
        ref={dropRef}
        className={newGroupClassName}
        onClick={handleCreateGroup}
      >
        {newDirectoryIcon}
        New group
      </button>

      <div className={s.ungroup}>
        {openDirectoryIcon}
        Ungroup
      </div>
    </div>
  );
});
