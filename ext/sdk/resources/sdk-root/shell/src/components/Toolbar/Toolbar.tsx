import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { BsCardHeading, BsHash, BsList } from 'react-icons/bs';
import { devtoolsIcon, newProjectIcon, openProjectIcon, projectBuildIcon, mapIcon, projectSettingsIcon } from 'constants/icons';
import { Project } from 'components/Project/Project';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { ProjectControls } from 'components/Project/ProjectControls/ProjectControls';
import { ToolbarTrigger } from './ToolbarTrigger';
import { Tour } from 'components/Tour/Tour';
import { ToolbarState } from 'store/ToolbarState';
import { ShellState } from 'store/ShellState';
import { ProjectState } from 'store/ProjectState';
import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';
import { StatusBar } from './StatusBar/StatusBar';
import s from './Toolbar.module.scss';
import { useOpenFlag } from 'utils/hooks';
import { Hasher } from './Hasher/Hasher';
import { Title } from 'components/controls/Title/Title';

const handleMenuClick = (openMenu) => openMenu();
const handleGetMenuCoords = () => ({
  top: 0,
  left: 0,
});

const useTour = () => {
  const [tourVisible, setTourVisible] = React.useState(false);

  React.useEffect(() => {
    const firstLaunch = localStorage.getItem('firstLaunch');

    if (!firstLaunch && ProjectState.hasProject) {
      setTourVisible(true);
      localStorage.setItem('firstLaunch', 'false');
    }
  }, [ProjectState.hasProject]);

  return {
    tourVisible,
    setTourVisible,
  };
};

export const Toolbar = observer(function Toolbar() {
  const { tourVisible, setTourVisible } = useTour();
  const [hasherOpen, openHasher, closeHasher] = useOpenFlag(false);

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
            icon: projectSettingsIcon,
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
    ContextMenuItemSeparator,
    {
      id: 'hasher',
      text: 'Hasher',
      icon: <BsHash />,
      onClick: openHasher,
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

  const projectToolbarClassName = classnames(s.pane, {
    [s.active]: ShellState.isTheia,
  });

  return (
    <div
      style={rootStyles}
      className={toolbarClasses}
      data-tour-id="toolbar"
    >
      <ToolbarTrigger />

      <div className={s['tools-bar']}>
        <div className={s.controls}>
          <ContextMenu
            items={contextMenuItems}
            onClick={handleMenuClick}
            getCoords={handleGetMenuCoords}
          >
            <Title animated={false} delay={0} title="Menu" fixedOn="bottom-left">
              {(ref) => (
                <button
                  ref={ref}
                  className={s.menu}
                >
                  <BsList />
                </button>
              )}
            </Title>
          </ContextMenu>

          <div className={s['project-name']} title={ProjectState.projectName}>
            <span>
              {ProjectState.projectName}
            </span>
          </div>

          {ProjectState.hasProject && ShellState.isTheia && (
            <ProjectControls />
          )}
        </div>

        <div className={projectToolbarClassName} title="">
          <ScrollContainer>
            <Project />
          </ScrollContainer>
        </div>

        <StatusBar />

        <Tour tourVisible={tourVisible} setTourVisible={setTourVisible} />

        {hasherOpen && (
          <Hasher close={closeHasher} />
        )}
      </div>
    </div>
  );
});
