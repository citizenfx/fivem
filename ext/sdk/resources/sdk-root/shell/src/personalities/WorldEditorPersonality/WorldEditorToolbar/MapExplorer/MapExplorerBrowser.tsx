import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { Indicator } from 'components/Indicator/Indicator';
import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';
import { BsMap } from 'react-icons/bs';
import { RiMapPinAddFill } from 'react-icons/ri';
import { openDirectoryIcon } from 'constants/icons';
import { MapExplorerItem } from './MapExplorerItem';
import s from './MapExplorer.module.scss';
import { ObjectsBrowserTrigger } from '../ObjectsBrowser/ObjectsBrowser';
import { Resizer } from 'components/controls/Resizer/Resizer';
import { useOpenFlag } from 'utils/hooks';

export const MapExplorerBrowser = observer(function MapExplorerBrowser({ explorerRef }: { explorerRef: React.RefObject<HTMLDivElement> }) {
  const [resizing, enableResizing, disableResizing] = useOpenFlag(false);

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
        {WorldEditorState.map === null && (
          <Indicator />
        )}

        {WorldEditorState.map !== null && (
          <>
            <div className={s.additions}>
              <header>
                <div className={s.icon}>
                  <RiMapPinAddFill />
                </div>
                <div className={s.name}>
                  Map additions
                </div>
              </header>

              <div className={s.children}>
                {Object.entries(WorldEditorState.map.additionsByGroups).map(([groupIndex, additions]) => {
                  return (
                    <div key={`group-${groupIndex}`} className={s.group}>
                      <header>
                        <div className={s.icon}>
                          {openDirectoryIcon}
                        </div>
                        <div className={s.name}>
                          {WorldEditorState.map.additionGroups[groupIndex]}
                        </div>
                      </header>

                      <div className={s.children}>
                        {Object.entries(additions).map(([id, obj]) => (
                          <MapExplorerItem
                            key={id}
                            item={obj}
                            labelPlaceholder="Addition label"
                            onLabelChange={(label: string) => label.trim() && WorldEditorState.map?.setAdditionLabel(id, label.trim())}
                            onClick={() => WorldEditorState.setCam(obj.cam)}
                          />
                        ))}
                      </div>
                    </div>
                  );
                })}

                {Object.entries(WorldEditorState.map.additionsUngrouped).map(([id, obj]) => (
                  <MapExplorerItem
                    key={id}
                    item={obj}
                    labelPlaceholder="Addition label"
                    onLabelChange={(label: string) => label.trim() && WorldEditorState.map?.setAdditionLabel(id, label.trim())}
                    onClick={() => WorldEditorState.setCam(obj.cam)}
                  />
                ))}
              </div>
            </div>

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
