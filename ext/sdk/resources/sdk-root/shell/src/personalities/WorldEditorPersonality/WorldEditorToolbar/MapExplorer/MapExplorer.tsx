import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { BiWorld } from 'react-icons/bi';
import { useOutsideClick } from 'utils/hooks';
import { MapExplorerBrowser } from './MapExplorerBrowser';
import s from './MapExplorer.module.scss';
import { ObjectsBrowser, ObjectsBrowserTrigger } from '../ObjectsBrowser/ObjectsBrowser';
import { HTML5Backend } from 'react-dnd-html5-backend';
import { DndProvider } from 'react-dnd';

function MapExplorerSentinel({ explorerRef }) {
  useOutsideClick(explorerRef, WorldEditorState.closeMapExplorer);

  return null;
}

export const MapExplorer = observer(function MapExplorer() {
  const explorerRef = React.useRef<HTMLDivElement>();

  const triggerClassName = classnames(s.trigger, {
    [s.active]: WorldEditorState.mapExplorerOpen,
  });

  return (
    <div className={s.root}>
      <div className={triggerClassName} onClick={WorldEditorState.toggleMapExplorer}>
        <div className={s.icon}>
          <BiWorld />
        </div>
        <div className={s.name}>
          <span>
            {WorldEditorState.mapName}
          </span>
        </div>
        <div className={s.controls}>
          <ObjectsBrowserTrigger />
        </div>
      </div>

      {WorldEditorState.mapExplorerOpen && (
        <MapExplorerSentinel explorerRef={explorerRef} />
      )}


      <DndProvider backend={HTML5Backend}>
        <MapExplorerBrowser explorerRef={explorerRef} />
      </DndProvider>

      {WorldEditorState.objectsBrowserOpen && (
        <ObjectsBrowser />
      )}
    </div>
  );
});
