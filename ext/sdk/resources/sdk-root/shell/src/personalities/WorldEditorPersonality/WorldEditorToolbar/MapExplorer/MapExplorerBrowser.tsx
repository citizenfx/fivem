import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { DropTargetMonitor, useDrop } from 'react-dnd';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { Indicator } from 'components/Indicator/Indicator';
import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';
import { BsMap } from 'react-icons/bs';
import { Resizer } from 'components/controls/Resizer/Resizer';
import { useOpenFlag } from 'utils/hooks';
import s from './MapExplorer.module.scss';
import { mapExplorerDNDTypes } from './MapExplorer.constants';
import { MapExplorerAdditions } from './MapExplorerAdditions/MapExplorerAdditions';

export const MapExplorerBrowser = observer(function MapExplorerBrowser({ explorerRef }: { explorerRef: React.RefObject<HTMLDivElement> }) {
  const mapIsOpen = WorldEditorState.map !== null;

  const [resizing, enableResizing, disableResizing] = useOpenFlag(false);

  const [{ isDropping }, dropRef] = useDrop({
    accept: mapExplorerDNDTypes.ADDITION,
    drop(item: unknown | { id: string, type: string }, monitor: DropTargetMonitor) {
      if (monitor.didDrop()) {
        return;
      }

      if (item['id']) {
        WorldEditorState.map.setAdditionGroup(item['id'], -1);
      }
    },
    collect: (monitor) =>({
      isDropping: monitor.isOver({ shallow: true }) && monitor.canDrop(),
    }),
  });

  const explorerClassName = classnames(s.explorer, {
    [s.active]: WorldEditorState.mapExplorerOpen,
    [s.resizing]: resizing,
  });

  const handleResize = React.useCallback((deltaWidth: number) => {
    WorldEditorState.setExplorerWidth(WorldEditorState.mapExplorerWidth + deltaWidth);
  }, []);

  return (
    <div ref={explorerRef} className={explorerClassName}>
      <Resizer
        onResize={handleResize}
        onResizingStart={enableResizing}
        onResizingStop={disableResizing}
      >
        {(handleMouseDown) => (
          <div className={s.resizer} onMouseDown={handleMouseDown} />
        )}
      </Resizer>

      <ScrollContainer className={s.content}>
        {!mapIsOpen && (
          <Indicator />
        )}

        {mapIsOpen && (
          <>
            <MapExplorerAdditions />

            <div className={s.patches}>
              <header>
                <div className={s.icon}>
                  <BsMap />
                </div>
                <div className={s.name}>
                  Map patches
                </div>
              </header>

              <div className={s.children}>
                {Object.values(WorldEditorState.map.patches).map((mapdata) => Object.entries(mapdata).map(([id, patch]) => {
                  return (
                    <div key={mapdata + id} className={s.item} onClick={() => WorldEditorState.setCam(patch.cam)}>
                      {patch.label}
                    </div>
                  );
                }))}
              </div>
            </div>
          </>
        )}
      </ScrollContainer>
    </div>
  );
});
