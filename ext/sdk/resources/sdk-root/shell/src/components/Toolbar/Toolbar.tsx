import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { BsCardHeading, BsGear, BsList } from 'react-icons/bs';
import { devtoolsIcon, newProjectIcon, openProjectIcon, projectBuildIcon, mapIcon } from 'constants/icons';
import { ProjectCreator } from 'components/Project/ProjectCreator/ProjectCreator';
import { ProjectOpener } from 'components/Project/ProjectOpener/ProjectOpener';
import { Project } from 'components/Project/Project';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { TaskReporter } from 'components/TaskReporter/TaskReporter';
import { ProjectToolbar } from 'components/Project/ProjectToolbar/ProjectToolbar';
import { ToolbarTrigger } from './ToolbarTrigger';
import { Tour } from 'components/Tour/Tour';
import { ToolbarState } from 'store/ToolbarState';
import { ShellState } from 'store/ShellState';
import { ProjectState } from 'store/ProjectState';
import s from './Toolbar.module.scss';

const handleMenuClick = (openMenu) => openMenu();
const handleGetMenuCoords = () => ({
  top: 0,
  left: 0,
});

export const Toolbar = observer(function Toolbar() {
  const [tourVisible, setTourVisible] = React.useState(false);

  const toolbarClasses = classnames(s.root, {
    [s.active]: ToolbarState.isOpen,
  });

  const contextMenuItems: ContextMenuItemsCollection = React.useMemo((): ContextMenuItemsCollection => [
    {
      id: 'open-project',
      text: 'Open Project',
      icon: openProjectIcon,
      onClick: ProjectState.openOpener,
    },
    {
      id: 'new-project',
      text: 'New Project',
      icon: newProjectIcon,
      onClick: ProjectState.openCreator,
    },
    ...(
      ProjectState.hasProject
        ? [
          ContextMenuItemSeparator,
          {
            id: 'project-settings',
            text: 'Project settings',
            icon: <BsGear />,
            onClick: ProjectState.openSettings,
          },
          {
            id: 'build-project',
            text: 'Build Project',
            icon: projectBuildIcon,
            onClick: ProjectState.openBuilder,
          },
        ] as ContextMenuItemsCollection
        : []
    ),
    ContextMenuItemSeparator,
    {
      id: 'changelog',
      text: 'Changelog',
      icon: <BsCardHeading />,
      onClick: ShellState.openChangelog,
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
  ], [ProjectState.hasProject]);

  const rootStyles: React.CSSProperties = {
    '--toolbar-width': `${ToolbarState.width}px`,
  } as any;

  React.useEffect(() => {
    // localStorage.removeItem('firstLaunch');
    const firstLaunch = localStorage.getItem('firstLaunch');
    if (!firstLaunch && ProjectState.hasProject) {
      setTourVisible(true);
      localStorage.setItem('firstLaunch', 'false');
    }
  }, [ProjectState.hasProject]);

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

          <div className={s['project-name']} title={ProjectState.projectName}>
            <span>
              {ProjectState.projectName}
            </span>
          </div>

          {ProjectState.hasProject && (
            <ProjectToolbar />
          )}
        </div>

        {ProjectState.creatorOpen && <ProjectCreator />}
        {ProjectState.openerOpen && <ProjectOpener />}

        <Project />

        <TaskReporter />
        <Tour tourVisible={tourVisible} setTourVisible={setTourVisible} />
      </div>
    </div>
  );
});
