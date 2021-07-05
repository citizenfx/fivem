import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { useOpenFlag } from 'utils/hooks';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { InPlaceInput } from 'components/controls/InPlaceInput/InPlaceInput';
import { ContextMenu, ContextMenuItemsCollection } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, renameIcon } from 'constants/icons';
import { useDrag } from 'react-dnd';
import { mapExplorerDNDTypes } from '../MapExplorer.constants';
import s from './MapExplorerAdditions.module.scss';

export interface MapExplorerItemProps {
  id: string,
  item: { label: string },
  onClick?(): void,
  onDelete?(): void,
  onLabelChange: (label: string) => void,
  children?: React.ReactNode,
}

export const MapExplorerAdditionsItem = observer(function MapExplorerAdditionsItem(props: MapExplorerItemProps) {
  const { id, item: { label }, onLabelChange, onClick, onDelete, children } = props;

  const [editing, enterEditing, exitEditing] = useOpenFlag(false);

  React.useEffect(() => {
    if (editing) {
      return WorldEditorState.overrideInput();
    }
  }, [editing]);

  const handleLabelChange = React.useCallback((newLabel: string) => {
    if (newLabel.trim()) {
      onLabelChange(newLabel.trim());
    }

    exitEditing();
  }, [exitEditing, onLabelChange]);

  const contextMenu = React.useMemo(() => [
    {
      id: 'edit',
      text: 'Rename addition',
      icon: renameIcon,
      onClick: enterEditing,
    },
    ...(
      onDelete
        ? [{
          id: 'delete',
          text: 'Delete addition',
          icon: deleteIcon,
          onClick: onDelete,
        }]
        : []
    ),
  ] as ContextMenuItemsCollection, [onDelete, enterEditing]);

  const [{ isDragging }, dragRef] = useDrag({
    item: { id, type: mapExplorerDNDTypes.ADDITION },
    canDrag: () => !editing,
    collect: (monitor) => ({
      isDragging: monitor.isDragging(),
    }),
  });

  const itemClassName = classnames(s.item, {
    [s.editing]: editing,
    [s.dragging]: isDragging,
  });

  return (
    <ContextMenu
      ref={dragRef}
      className={itemClassName}
      activeClassName={s.active}
      onClick={onClick}
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
