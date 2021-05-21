import React from 'react';
import { ProjectItemProps } from "components/Project/ProjectExplorer/item";
import { itemsStyles } from "components/Project/ProjectExplorer/item.styles";
import { observer } from "mobx-react-lite";
import { BiWorld } from "react-icons/bi";
import { FXWORLD_FILE_EXT } from '../fxworld-types';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';

export const FXWorld = observer(function FXWorld(props: ProjectItemProps) {
  const assetPath = props.entry.path;

  const handleOpen = React.useCallback(() => {
    WorldEditorState.openMap(props.entry);
  }, [assetPath]);

  return (
    <div className={itemsStyles.wrapper}>
      <div className={itemsStyles.item} onClick={handleOpen}>
        <div className={itemsStyles.itemIcon}>
          <BiWorld />
        </div>
        <div className={itemsStyles.itemTitle}>
          {props.entry.name.replace(FXWORLD_FILE_EXT, '')}
        </div>
      </div>
    </div>
  );
});
