import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { useOpenFlag } from 'utils/hooks';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { InPlaceInput } from 'components/controls/InPlaceInput/InPlaceInput';
import { ContextMenu, ContextMenuItemsCollection } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, renameIcon } from 'constants/icons';
import { useDrag } from 'react-dnd';
import s from './AdditionsTool.module.scss';
import { ADDITION_DND_TYPES } from './AdditionsTool.constants';
import { WESelectionType } from 'backend/world-editor/world-editor-types';

export interface AdditionProps {
  id: string,
  children?: React.ReactNode,
}

export const Addition = observer(function Addition(props: AdditionProps) {
  const { id, children } = props;

  const { label } = WEState.map?.additions[id] || { label: '' };

  const [editing, enterEditing, exitEditing] = useOpenFlag(false);

  const handleClick = React.useCallback(() => {
    WEState.setEditorSelection({
      type: WESelectionType.ADDITION,
      id,
    });
  }, [id]);

  const handleLabelChange = React.useCallback((newLabel: string) => {
    if (newLabel.trim()) {
      WEState.map?.setAdditionLabel(id, newLabel.trim());
    }

    exitEditing();
  }, [id, exitEditing]);

  const contextMenu = React.useMemo(() => [
    {
      id: 'edit',
      text: 'Rename addition',
      icon: renameIcon,
      onClick: enterEditing,
    },
    {
      id: 'delete',
      text: 'Delete addition',
      icon: deleteIcon,
      onClick: () => WEState.map?.deleteAddition(id),
    },
  ] as ContextMenuItemsCollection, [id, enterEditing]);

  const [{ isDragging }, dragRef] = useDrag({
    item: { id, type: ADDITION_DND_TYPES.ADDITION },
    canDrag: () => !editing,
    collect: (monitor) => ({
      isDragging: monitor.isDragging(),
    }),
  });

  const itemClassName = classnames(s.item, {
    [s.editing]: editing,
    [s.dragging]: isDragging,
    [s.highlight]: WEState.isAdditionSelected(id),
  });

  return (
    <ContextMenu
      ref={dragRef}
      className={itemClassName}
      activeClassName={s.active}
      onClick={handleClick}
      onDoubleClick={enterEditing}
      items={contextMenu}
    >
      {!editing && (
        <>
          <div className={s.label}>
            {label}
          </div>

          {!!children && (
            <div className={s.controls}>
              {children}
            </div>
          )}
        </>
      )}

      {editing && (
        <InPlaceInput
          value={label}
          onChange={handleLabelChange}
          placeholder="Addition label"
        />
      )}
    </ContextMenu>
  );
});
