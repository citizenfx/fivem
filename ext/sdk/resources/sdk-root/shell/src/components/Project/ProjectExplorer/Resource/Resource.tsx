import React from 'react';
import classnames from 'classnames';
import { BsCheckBox, BsSquare } from 'react-icons/bs';
import { FilesystemEntry, ServerStates } from 'shared/api.types';
import { ServerContext } from 'contexts/ServerContext';
import { useOpenFlag } from 'utils/hooks';
import { sendApiMessage } from 'utils/api';
import { projectApi, serverApi } from 'shared/events';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, disabledResourceIcon, enabledResourceIcon, refreshIcon, renameIcon, resourceIcon, startIcon, stopIcon } from 'constants/icons';
import { useExpandablePath, useItem, useItemDrop, useItemRelocateTargetContextMenu } from '../ProjectExplorer.hooks';
import { ProjectItemProps, renderChildren } from '../ProjectExplorer.item';
import { ProjectExplorerItemContext } from '../ProjectExplorer.itemContext';
import { projectExplorerItemType } from '../ProjectExplorer.itemTypes';
import { ResourceDeleter } from './ResourceDeleter/ResourceDeleter';
import { ResourceRenamer } from './ResourceRenamer/ResourceRenamer';
import s from './Resource.module.scss';


const resourceChildrenFilter = (entry: FilesystemEntry) => {
  if (entry.name === 'fxasset.json') {
    return false;
  }

  return true;
};

export interface ResourceProps {
  name: string,
  path: string,
};

export const Resource = React.memo(function Resource(props: ProjectItemProps) {
  const { entry, project, projectResources } = props;
  const projectResource = projectResources[entry.path];

  const { serverState, resourcesState } = React.useContext(ServerContext);
  const options = React.useContext(ProjectExplorerItemContext);

  const { renderItemControls, contextMenuItems } = useItem(props);
  const { expanded, toggleExpanded } = useExpandablePath(entry.path);

  const isEnabled = !!projectResource?.enabled;
  const isAutorestartOnChangeEnabled = !!projectResource?.restartOnChange;

  const [deleterOpen, openDeleter, closeDeleter] = useOpenFlag(false);
  const [renamerOpen, openRenamer, closeRenamer] = useOpenFlag(false);

  const handleToggleEnabled = React.useCallback(() => {
    sendApiMessage(projectApi.setResourceEnabled, {
      resourceName: entry.name,
      enabled: !isEnabled,
    });
  }, [entry, project, isEnabled]);
  const handleToggleAutorestartEnabled = React.useCallback(() => {
    sendApiMessage(projectApi.setResourceConfig, {
      resourceName: entry.name,
      config: {
        restartOnChange: !isAutorestartOnChangeEnabled,
      },
    });
  }, [entry, isAutorestartOnChangeEnabled]);

  const handleRestart = React.useCallback(() => {
    sendApiMessage(serverApi.restartResource, entry.name);
  }, [entry]);
  const handleStop = React.useCallback(() => {
    sendApiMessage(serverApi.stopResource, entry.name);
  }, [entry]);
  const handleStart = React.useCallback(() => {
    sendApiMessage(serverApi.startResource, entry.name);
  }, [entry]);

  const relocateTargetContextMenu = useItemRelocateTargetContextMenu(entry);

  const itemContextMenuItems: ContextMenuItemsCollection = React.useMemo(() => {
    const serverIsDown = serverState !== ServerStates.up;

    const serverRelatedItems: ContextMenuItemsCollection = [];

    if (resourcesState[entry.name]) {
      serverRelatedItems.push({
        id: 'stop',
        icon: stopIcon,
        text: 'Stop',
        disabled: serverIsDown,
        onClick: handleStop,
      }, {
        id: 'restart',
        icon: refreshIcon,
        text: 'Restart',
        disabled: serverIsDown,
        onClick: handleRestart,
      });
    } else {
      serverRelatedItems.push({
        id: 'Start',
        icon: startIcon,
        text: 'Start',
        disabled: serverIsDown,
        onClick: handleStart,
      });
    }

    return [
      ...serverRelatedItems,
      ContextMenuItemSeparator,
      {
        id: 'toggle-enabled',
        icon: isEnabled
          ? disabledResourceIcon
          : enabledResourceIcon,
        text: isEnabled
          ? 'Disable resource'
          : 'Enable resource',
        onClick: handleToggleEnabled,
      },
      {
        id: 'toggle-autorestart-enabled',
        icon: isAutorestartOnChangeEnabled
          ? <BsSquare />
          : <BsCheckBox />,
        text: isAutorestartOnChangeEnabled
          ? 'Disable restart on change'
          : 'Enable restart on change',
        onClick: handleToggleAutorestartEnabled,
      },
      ContextMenuItemSeparator,
      ...relocateTargetContextMenu,
      ContextMenuItemSeparator,
      {
        id: 'delete',
        icon: deleteIcon,
        text: 'Delete resource',
        disabled: options.disableAssetDelete,
        onClick: openDeleter,
      },
      {
        id: 'rename',
        icon: renameIcon,
        text: 'Rename resource',
        disabled: options.disableAssetRename,
        onClick: openRenamer,
      },
      ContextMenuItemSeparator,
      ...contextMenuItems,
    ];
  }, [
    serverState,
    resourcesState,
    contextMenuItems,
    options,
    isEnabled,
    isAutorestartOnChangeEnabled,
    handleToggleAutorestartEnabled,
    handleToggleEnabled,
    openDeleter,
    openRenamer,
    relocateTargetContextMenu,
  ]);

  const iconTitle = projectResource?.enabled
    ? 'Enabled'
    : 'Disabled';

  const iconsClassName = classnames(s.icon, {
    [s.active]: resourcesState[entry.name],
  });

  const icon = projectResource?.enabled
    ? enabledResourceIcon
    : resourceIcon;

  const children = renderChildren(entry, props, resourceChildrenFilter);

  const { isDropping, dropRef } = useItemDrop(entry, [
    projectExplorerItemType.FILE,
    projectExplorerItemType.FOLDER,
  ]);

  const rootClassName = classnames(s.root, {
    [s.dropping]: isDropping,
  })

  return (
    <div className={rootClassName} ref={dropRef}>
      <ContextMenu
        className={s.resource}
        activeClassName={s.active}
        items={itemContextMenuItems}
        onClick={toggleExpanded}
      >
        <div className={s.name}>
          <span className={iconsClassName} title={iconTitle}>
            {icon}
          </span>
          {entry.name}
        </div>
      </ContextMenu>

      {expanded && (
        <div className={s.children}>
          {renderItemControls()}
          {children}
        </div>
      )}

      {deleterOpen && (
        <ResourceDeleter path={entry.path} name={entry.name} onClose={closeDeleter} />
      )}
      {renamerOpen && (
        <ResourceRenamer path={entry.path} name={entry.name} onClose={closeRenamer} />
      )}
    </div>
  );
});
