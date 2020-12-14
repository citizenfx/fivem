import React from 'react';
import { BsArrowBarLeft, BsArrowBarRight, BsCardHeading, BsList } from 'react-icons/bs';
import classnames from 'classnames';
import { StateContext } from 'contexts/StateContext';
import { ProjectContext } from 'contexts/ProjectContext';
import { AppStates } from 'shared/api.types';
import { devtoolsIcon, newProjectIcon, openProjectIcon } from 'constants/icons';
import { ProjectCreator } from 'components/Project/ProjectCreator/ProjectCreator';
import { ProjectOpener } from 'components/Project/ProjectOpener/ProjectOpener';
import { Project } from 'components/Project/Project';
import s from './Toolbar.module.scss';
import { ContextMenu, ContextMenuItemsCollection } from 'components/controls/ContextMenu/ContextMenu';


export const Toolbar = React.memo(function Toolbar() {
  const { state, toolbarOpen, openToolbar, closeToolbar, openChangelog } = React.useContext(StateContext);
  const { openCreator, openOpener, creatorOpen, openerOpen } = React.useContext(ProjectContext);

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
    [s.visible]: state === AppStates.ready,
    [s.active]: toolbarOpen,
  });

  const contextMenuItems: ContextMenuItemsCollection = React.useMemo((): ContextMenuItemsCollection => [
    {
      id: 'changelog',
      text: 'Changelog',
      icon: <BsCardHeading />,
      onClick: openChangelog,
    },
    {
      id: 'dev-tools',
      text: 'DevTools',
      icon: devtoolsIcon,
      onClick: () => window.openDevTools(),
    },
  ], []);

  const triggerTitle = toolbarOpen
    ? 'Collapse FxDK toolbar'
    : 'Expand FxDK toolbar';
  const triggerIcon = toolbarOpen
    ? <BsArrowBarLeft />
    : <BsArrowBarRight />;

  return (
    <div
      className={toolbarClasses}
    >
      <div className={s.backdrop} onClick={closeToolbar} />

      <button className={s.trigger} onClick={toggleToolbar} title={triggerTitle}>
        {triggerIcon}
      </button>

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


          <ContextMenu items={contextMenuItems} onClick={(openMenu) => openMenu()}>
            <button style={{ flexGrow: 0 }} title="Open DevTools">
              <BsList />
            </button>
          </ContextMenu>
        </div>

        {creatorOpen && <ProjectCreator />}
        {openerOpen && <ProjectOpener />}

        <Project />
      </div>
    </div>
  );
});
