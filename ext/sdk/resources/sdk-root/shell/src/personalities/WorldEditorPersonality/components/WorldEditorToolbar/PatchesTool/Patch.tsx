import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { ContextMenu, ContextMenuItemsCollection } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, renameIcon } from 'constants/icons';
import s from './PatchesTool.module.scss';
import { WESelectionType } from 'backend/world-editor/world-editor-types';
import { useOpenFlag } from 'utils/hooks';
import { InPlaceInput } from 'components/controls/InPlaceInput/InPlaceInput';

export interface PatchProps {
  mapData: string,
  entityId: string,
}

export const Patch = observer(function Patch(props: PatchProps) {
  const { mapData, entityId } = props;
  const patch = WEState.map!.patches[mapData][entityId];

  const [editing, enterEditing, exitEditing] = useOpenFlag(false);
  const handleLabelChange = React.useCallback((newLabel: string) => {
    if (newLabel.trim()) {
      WEState.map?.setPatchLabel(mapData, entityId, newLabel.trim());
    }

    exitEditing();
  }, [mapData, entityId, exitEditing]);

  const contextMenu = React.useMemo(() => [
    {
      id: 'edit',
      text: 'Rename addition',
      icon: renameIcon,
      onClick: enterEditing,
    },
    {
      id: 'delete',
      icon: deleteIcon,
      text: 'Delete patch',
      onClick: () => WEState.map!.deletePatch(mapData, entityId),
    }
  ] as ContextMenuItemsCollection, [mapData, entityId, enterEditing]);

  const itemClassName = classnames(s.item, {
    [s.editing]: editing,
    [s.highlight]: WEState.isPatchSelected(mapData, entityId),
  });

  const handleClick = () => {
    WEState.setEditorSelection({
      type: WESelectionType.PATCH,
      label: patch.label,
      mapdata: parseInt(mapData, 10),
      entity: parseInt(entityId, 10),
    });
  };

  return (
    <ContextMenu
      items={contextMenu}
      className={itemClassName}
      activeClassName={s.active}
      onClick={handleClick}
      onDoubleClick={enterEditing}
    >
      {!editing && patch.label}

      {editing && (
        <InPlaceInput
          value={patch.label}
          onChange={handleLabelChange}
          placeholder="Patch label"
        />
      )}
    </ContextMenu>
  );
});
