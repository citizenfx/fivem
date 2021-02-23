import React from 'react';
import classnames from 'classnames';
import { BsCheckBox, BsFillEyeFill, BsSquare } from 'react-icons/bs';
import { ServerStates } from 'shared/api.types';
import { ServerContext } from 'contexts/ServerContext';
import { useOpenFlag } from 'utils/hooks';
import { sendApiMessage } from 'utils/api';
import { projectApi, serverApi } from 'shared/api.events';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, disabledResourceIcon, enabledResourceIcon, refreshIcon, renameIcon, resourceIcon, startIcon, stopIcon } from 'constants/icons';
import { useExpandablePath, useItem, useItemDrop, useItemRelocateTargetContextMenu } from '../ProjectExplorer.hooks';
import { ProjectItemProps, renderChildren } from '../item';
import { ProjectExplorerItemContext, ProjectExplorerItemContextProvider } from '../item.context';
import { projectExplorerItemType } from '../item.types';
import { ResourceDeleter } from './ResourceDeleter/ResourceDeleter';
import { ResourceRenamer } from './ResourceRenamer/ResourceRenamer';
import { ProjectSetResourceConfigRequest } from 'shared/api.requests';
import { ResourceStatus } from 'backend/project/asset/asset-contributions/resource/resource-types';
import { useStatus } from 'contexts/StatusContext';
import { ResourceCommandsOutputModal } from './ResourceCommandsOutputModal/ResourceCommandsOutputModal';
import { itemsStyles } from '../item.styles';
import { NativeTypes } from 'react-dnd-html5-backend';
import { ProjectResource } from 'shared/project.types';
import s from './Resource.module.scss';


const contextOptions: Partial<ProjectExplorerItemContext> = {
  disableAssetCreate: true,
};

const defaultResourceStatus: ResourceStatus = {
  watchCommands: {},
};

export interface ResourceProps {
  name: string,
  path: string,
};

export const Resource = React.memo(function Resource(props: ProjectItemProps) {
  const { entry, project } = props;

  const resourceConfig = project.resources[entry.name];
  const resourceStatus = useStatus<ResourceStatus>(`resource-${entry.path}`, defaultResourceStatus);

  const options = React.useContext(ProjectExplorerItemContext);

  const { renderItemControls, contextMenuItems, requiredContextMenuItems } = useItem(props);
  const { expanded, toggleExpanded } = useExpandablePath(entry.path);

  const [deleterOpen, openDeleter, closeDeleter] = useOpenFlag(false);
  const [renamerOpen, openRenamer, closeRenamer] = useOpenFlag(false);

  const {
    resourceIsRunning,
    resourceIsEnabled,
    lifecycleContextMenuItems,
    commandsOutputOpen,
    closeCommandsOutput,
  } = useResourceLifecycle(entry.name, resourceConfig, resourceStatus);

  const relocateTargetContextMenu = useItemRelocateTargetContextMenu(entry);

  const itemContextMenuItems: ContextMenuItemsCollection = React.useMemo(() => {
    return [
      ...lifecycleContextMenuItems,
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
      ContextMenuItemSeparator,
      ...requiredContextMenuItems,
    ];
  }, [
    lifecycleContextMenuItems,
    contextMenuItems,
    options,
    openDeleter,
    openRenamer,
    relocateTargetContextMenu,
    requiredContextMenuItems,
  ]);

  const children = renderChildren(entry, { ...props, childrenCollapsed: true });

  const { isDropping, dropRef } = useItemDrop(entry, [
    projectExplorerItemType.FILE,
    projectExplorerItemType.FOLDER,
    NativeTypes.FILE,
  ]);

  const rootClassName = classnames(itemsStyles.wrapper, {
    [itemsStyles.dropping]: isDropping,
  })

  return (
    <div className={rootClassName} ref={dropRef}>
      <ContextMenu
        className={itemsStyles.item}
        activeClassName={itemsStyles.itemActive}
        items={itemContextMenuItems}
        onClick={toggleExpanded}
      >
        <ResourceIcon
          enabled={resourceIsEnabled}
          running={resourceIsRunning}
        />
        <div className={itemsStyles.itemTitle} title={entry.name}>
          {entry.name}
        </div>
        <ResourceStatusNode
          resourceStatus={resourceStatus}
        />
      </ContextMenu>

      {expanded && (
        <div className={itemsStyles.children}>
          {renderItemControls()}
          <ProjectExplorerItemContextProvider options={contextOptions}>
            {children}
          </ProjectExplorerItemContextProvider>
        </div>
      )}

      {deleterOpen && (
        <ResourceDeleter path={entry.path} name={entry.name} onClose={closeDeleter} />
      )}
      {renamerOpen && (
        <ResourceRenamer path={entry.path} name={entry.name} onClose={closeRenamer} />
      )}

      {commandsOutputOpen && (
        <ResourceCommandsOutputModal
          onClose={closeCommandsOutput}
          resourceName={entry.name}
          resourceStatus={resourceStatus}
        />
      )}
    </div>
  );
});

interface ResourceStatusNodeProps {
  resourceStatus: ResourceStatus,
}
const ResourceStatusNode = React.memo(function ResourceStatusNode({ resourceStatus }: ResourceStatusNodeProps) {
  const watchCommands = Object.values(resourceStatus.watchCommands);
  const watchCommandsNode = watchCommands.length
    ? (
      <div className={itemsStyles.itemStatusEntry} title="Watch commands: running / declared">
        <BsFillEyeFill /> {watchCommands.filter((cmd) => cmd.running).length}/{watchCommands.length}
      </div>
    )
    : null;

  return (
    <div className={itemsStyles.itemStatus}>
      {watchCommandsNode}
    </div>
  );
});

const ResourceIcon = React.memo(function ResourceIcon({ enabled, running }: { enabled: boolean, running: boolean }) {
  const icon = enabled
    ? enabledResourceIcon
    : resourceIcon;

  const iconTitle = enabled
    ? 'Enabled'
    : 'Disabled';

  const iconsClassName = classnames(itemsStyles.itemIcon, {
    [s.running]: running,
  });

  return (
    <span className={iconsClassName} title={iconTitle}>
      {icon}
    </span>
  );
});

function useResourceLifecycle(resourceName: string, resourceConfig: ProjectResource, resourceStatus: ResourceStatus) {
  const { serverState, resourcesState } = React.useContext(ServerContext);

  const [commandsOutputOpen, openCommandsOutput, closeCommandsOutput] = useOpenFlag(false);

  const isEnabled = !!resourceConfig?.enabled;
  const isAutorestartOnChangeEnabled = !!resourceConfig?.restartOnChange;

  const handleToggleEnabled = React.useCallback(() => {
    const request: ProjectSetResourceConfigRequest = {
      resourceName,
      config: {
        enabled: !isEnabled,
      },
    };

    sendApiMessage(projectApi.setResourceConfig, request);
  }, [resourceName, isEnabled]);

  const handleToggleAutorestartEnabled = React.useCallback(() => {
    const request: ProjectSetResourceConfigRequest = {
      resourceName,
      config: {
        restartOnChange: !isAutorestartOnChangeEnabled,
      },
    };

    sendApiMessage(projectApi.setResourceConfig, request);
  }, [resourceName, isAutorestartOnChangeEnabled]);

  const handleRestart = React.useCallback(() => {
    sendApiMessage(serverApi.restartResource, resourceName);
  }, [resourceName]);
  const handleStop = React.useCallback(() => {
    sendApiMessage(serverApi.stopResource, resourceName);
  }, [resourceName]);
  const handleStart = React.useCallback(() => {
    sendApiMessage(serverApi.startResource, resourceName);
  }, [resourceName]);

  const lifecycleContextMenuItems: ContextMenuItemsCollection = React.useMemo(() => {
    const serverIsDown = serverState !== ServerStates.up;

    const serverRelatedItems: ContextMenuItemsCollection = [];

    if (resourcesState[resourceName]) {
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

    const fxdkCommandsItems: ContextMenuItemsCollection = [];
    if (Object.values(resourceStatus.watchCommands).length) {
      fxdkCommandsItems.push(ContextMenuItemSeparator, {
        id: 'open-watch-commands-output',
        icon: <BsFillEyeFill />,
        text: 'Watch commands output',
        onClick: openCommandsOutput,
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
      ...fxdkCommandsItems,
    ];
  }, [
    resourceName,
    resourceStatus,
    resourcesState,
    isEnabled,
    isAutorestartOnChangeEnabled,
    handleStop,
    handleRestart,
    handleStart,
    openCommandsOutput,
  ]);

  return {
    resourceIsEnabled: isEnabled,
    resourceIsRunning: !!resourcesState[resourceName],
    lifecycleContextMenuItems,
    commandsOutputOpen,
    closeCommandsOutput,
  };
}
