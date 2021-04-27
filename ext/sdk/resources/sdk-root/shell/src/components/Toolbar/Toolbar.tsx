import React, { useState } from 'react';
import { BsCardHeading, BsGear, BsList } from 'react-icons/bs';
import classnames from 'classnames';
import { StateContext } from 'contexts/StateContext';
import { ProjectContext } from 'contexts/ProjectContext';
import { devtoolsIcon, newProjectIcon, openProjectIcon, projectBuildIcon, mapIcon } from 'constants/icons';
import { ProjectCreator } from 'components/Project/ProjectCreator/ProjectCreator';
import { ProjectOpener } from 'components/Project/ProjectOpener/ProjectOpener';
import { Project } from 'components/Project/Project';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { TaskReporter } from 'components/TaskReporter/TaskReporter';
import { ProjectToolbar } from 'components/Project/ProjectToolbar/ProjectToolbar';
import { ToolbarTrigger } from './ToolbarTrigger';
import { Tour } from 'components/Tour/Tour';
import s from './Toolbar.module.scss';

const handleMenuClick = (openMenu) => openMenu();
const handleGetMenuCoords = () => ({
  top: 0,
  left: 0,
});

export const Toolbar = React.memo(function Toolbar() {
  const { toolbarWidth, toolbarOpen, openChangelog } = React.useContext(StateContext);
  const { project, openCreator, openOpener, creatorOpen, openerOpen, openSettings, openBuilder } = React.useContext(ProjectContext);
  const [tourVisible, setTourVisible] = useState(false);
  // const newResourceRef = useRef(null)

  const handleOpenCreator = React.useCallback(() => {
    openCreator();
  }, [openCreator]);
  const handleOpenOpener = React.useCallback(() => {
    openOpener();
  }, [openOpener]);

  const handleOpenSettings = React.useCallback(() => {
    openSettings();
  }, [openSettings]);

  const toolbarClasses = classnames(s.root, {
    [s.active]: toolbarOpen,
  });

  const contextMenuItems: ContextMenuItemsCollection = React.useMemo((): ContextMenuItemsCollection => [
    {
      id: 'open-project',
      text: 'Open Project',
      icon: openProjectIcon,
      onClick: handleOpenOpener,
    },
    {
      id: 'new-project',
      text: 'New Project',
      icon: newProjectIcon,
      onClick: handleOpenCreator,
    },
    ...(
      project
        ? [
          ContextMenuItemSeparator,
          {
            id: 'project-settings',
            text: 'Project settings',
            icon: <BsGear />,
            onClick: handleOpenSettings,
          },
          {
            id: 'build-project',
            text: 'Build Project',
            icon: projectBuildIcon,
            onClick: openBuilder,
          },
        ] as ContextMenuItemsCollection
        : []
    ),
    ContextMenuItemSeparator,
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
    ContextMenuItemSeparator,
    {
      id: 'tour',
      text: 'Tour',
      icon: mapIcon,
      onClick: () => setTourVisible(true),
    },
  ], [project, openBuilder]);

  const projectTitle = project?.manifest.name || 'No project open yet';

  const rootStyles: React.CSSProperties = {
    '--toolbar-width': `${toolbarWidth}px`,
  } as any;

  React.useEffect(() => {
    // localStorage.removeItem('firstLaunch');
    const firstLaunch = localStorage.getItem('firstLaunch');
    if (!firstLaunch && project) {
      setTourVisible(true);
      localStorage.setItem('firstLaunch', 'false');
    }
  }, [project]);

  return (
    <div
      style={rootStyles}
      className={toolbarClasses}
      title="Toolbar"
    >
      <ToolbarTrigger />

      <div className={s.bar}>
        <div className={s.controls}>
          <ContextMenu
            items={contextMenuItems}
            onClick={handleMenuClick}
            getCoords={handleGetMenuCoords}
          >
            <button title="FxDK Menu">
              <BsList />
            </button>
          </ContextMenu>

          <div className={s['project-name']} title={projectTitle}>
            <span>
              {projectTitle}
            </span>
          </div>

          {!!project && (
            <ProjectToolbar />
          )}
        </div>

        {creatorOpen && <ProjectCreator />}
        {openerOpen && <ProjectOpener />}

        <Project />

        <TaskReporter />
        <Tour tourVisible={tourVisible} setTourVisible={setTourVisible} />
      </div>
    </div>
  );
});
