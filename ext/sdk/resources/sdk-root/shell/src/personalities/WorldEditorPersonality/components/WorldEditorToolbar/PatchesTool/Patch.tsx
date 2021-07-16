import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { ContextMenu, ContextMenuItemsCollection } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon } from 'constants/icons';
import s from './PatchesTool.module.scss';

export interface PatchProps {
  mapData: string,
  entityId: string,
}

export const Patch = observer(function Patch(props: PatchProps) {
  const { mapData, entityId } = props;
  const patch = WEState.map.patches[mapData][entityId];

  const contextMenu = React.useMemo(() => [
    {
      id: 'delete',
      icon: deleteIcon,
      text: 'Delete patch',
      onClick: () => WEState.map.deletePatch(mapData, entityId),
    }
  ] as ContextMenuItemsCollection, [mapData, entityId]);

  const itemClassName = classnames(s.item, {
    [s.highlight]: WEState.isPatchSelected(mapData, entityId),
  });

  return (
    <ContextMenu
      items={contextMenu}
      className={itemClassName}
      activeClassName={s.active}
      onClick={() => WEState.setCam(patch.cam)}
    >
      {patch.label}
    </ContextMenu>
  );
});
