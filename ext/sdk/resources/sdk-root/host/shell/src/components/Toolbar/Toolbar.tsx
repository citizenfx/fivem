import React from 'react';
import classnames from 'classnames';
import { StateContext } from '../../contexts/StateContext';
import { Project } from '../Project/Project';
import { ProjectCreator } from '../Project/ProjectCreator/ProjectCreator';
import { ProjectContext } from '../../contexts/ProjectContext';
import { ProjectOpener } from '../Project/ProjectOpener/ProjectOpener';
import { devtoolsIcon, newProjectIcon, openProjectIcon } from '../../constants/icons';
import { States } from '../../sdkApi/api.types';
import s from './Toolbar.module.scss';


export const Toolbar = React.memo(function Toolbar() {
  const { state, toolbarOpen, openToolbar, closeToolbar } = React.useContext(StateContext);
  const { openCreator, openOpener, creatorOpen, openerOpen } = React.useContext(ProjectContext);

  const handleOpenDevtools = React.useCallback(() => {
    window.openDevTools();
  }, []);

  const handleOpenCreator = React.useCallback(() => {
    openCreator();
  }, [openCreator]);
  const handleOpenOpener = React.useCallback(() => {
    openOpener();
  }, [openOpener]);

  const toggleToolbar = toolbarOpen
    ? closeToolbar
    : openToolbar;

  const toolbarClasses = classnames(s.root, {
    [s.visible]: state === States.ready,
    [s.active]: toolbarOpen,
  });

  return (
    <div
      className={toolbarClasses}
    >
      <div className={s.backdrop} onClick={closeToolbar} />

      <button className={s.trigger} onClick={toggleToolbar}>FxDK</button>

      <div className={s.bar}>
        <div className={s.controls}>
          <button onClick={handleOpenCreator} style={{ flexGrow: 1 }}>
            {newProjectIcon}
              New Project
            </button>
          <button onClick={handleOpenOpener} style={{ flexGrow: 1 }}>
            {openProjectIcon}
              Open Project
            </button>
          <button onClick={handleOpenDevtools} style={{ flexGrow: 0 }} title="Open DevTools">
            {devtoolsIcon}
          </button>
        </div>

        {creatorOpen && <ProjectCreator />}
        {openerOpen && <ProjectOpener />}

        <Project />
      </div>
    </div>
  );
});
