import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import s from './MapExplorer.module.scss';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { BiWorld } from 'react-icons/bi';
import { useOutsideClick } from 'utils/hooks';
import { Indicator } from 'components/Indicator/Indicator';
import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';
import { BsMap, BsPlus } from 'react-icons/bs';
import { RiMapPinAddFill } from 'react-icons/ri';
import { openDirectoryIcon } from 'constants/icons';
import { MapExplorerItem } from './MapExplorerItem';

const MapExplorerBrowser = observer(function MapExplorerBrowser() {
  const ref = React.useRef();

  useOutsideClick(ref, WorldEditorState.closeMapExplorer);

  return (
    <div ref={ref} className={s.explorer}>
      <ScrollContainer>
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

                <div className={s.controls}>
                  {/* <button>
                    <BsPlus />
                  </button> */}
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

export const MapExplorer = observer(function MapExplorer() {
  const triggerClassName = classnames(s.trigger, {
    [s.active]: WorldEditorState.mapExplorerOpen,
  });

  return (
    <div className={s.root}>
      <div className={triggerClassName} onClick={WorldEditorState.toggleMapExplorer}>
        <BiWorld />
        {WorldEditorState.mapName}
      </div>

      {WorldEditorState.mapExplorerOpen && (
        <MapExplorerBrowser />
      )}
    </div>
  );
});
