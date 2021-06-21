import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import s from './MapExplorer.module.scss';
import { useOpenFlag } from 'utils/hooks';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { InPlaceInput } from 'components/controls/InPlaceInput/InPlaceInput';

export interface MapExplorerItemProps {
  item: { label: string },
  onClick?: () => void,
  onLabelChange: (label: string) => void,
  labelPlaceholder: string,
  children?: React.ReactNode,
}

export const MapExplorerItem = observer(function MapExplorerItem(props: MapExplorerItemProps) {
  const { item: { label }, onLabelChange, onClick, labelPlaceholder, children } = props;

  console.log('RENDERING ITEM', label);

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

  const itemClassName = classnames(s.item, {
    [s.editing]: editing,
  });

  return (
    <div
      className={itemClassName}
      onClick={onClick}
      onDoubleClick={enterEditing}
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
          placeholder={labelPlaceholder}
        />
      )}
    </div>
  );
});
