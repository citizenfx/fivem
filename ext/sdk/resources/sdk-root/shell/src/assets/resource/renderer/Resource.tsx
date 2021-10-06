import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { NativeTypes } from 'react-dnd-html5-backend';
import { BsExclamationCircle, BsFillEyeFill } from 'react-icons/bs';
import { useOpenFlag } from 'utils/hooks';
import { projectApi, serverApi } from 'shared/api.events';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'fxdk/ui/controls/ContextMenu/ContextMenu';
import { checkedIcon, deleteIcon, disabledResourceIcon, enabledResourceIcon, refreshIcon, renameIcon, resourceIcon, startIcon, stopIcon, uncheckedIcon } from 'constants/icons';
import { useExpandablePath, useItem, useItemDragAndDrop, useItemRelocateTargetContextMenu } from 'fxdk/project/browser/ProjectExplorer/ProjectExplorer.hooks';
import { ProjectExplorerItemContext, ProjectExplorerItemContextProvider } from 'fxdk/project/browser/ProjectExplorer/item.context';
import { ResourceRenamer } from './ResourceRenamer/ResourceRenamer';
import { APIRQ } from 'shared/api.requests';
import { StatusState } from 'store/StatusState';
import { ResourceAssetConfig, ResourceStatus } from 'assets/resource/resource-types';
import { ServerState } from 'store/ServerState';
import { ResourceCommandsOutputModal } from './ResourceCommandsOutputModal/ResourceCommandsOutputModal';
import { itemsStyles } from 'fxdk/project/browser/ProjectExplorer/item.styles';
import { ProjectItemProps } from 'fxdk/project/browser/ProjectExplorer/item';
import { FilesystemEntry } from 'shared/api.types';
import { ProjectState } from 'store/ProjectState';
import s from './Resource.module.scss';
import { SystemResource, SYSTEM_RESOURCES_NAMES } from 'backend/system-resources/system-resources-constants';
import { ItemState } from 'fxdk/project/browser/ProjectExplorer/ItemState';
import { Title } from 'fxdk/ui/controls/Title/Title';
import mergeRefs from 'utils/mergeRefs';
import { projectExplorerItemType } from 'fxdk/project/browser/ProjectExplorer/item.types';
import { Api } from 'fxdk/browser/Api';


const contextOptions: Partial<ProjectExplorerItemContext> = {
  disableAssetCreate: true,
};

const defaultResourceConfig: ResourceAssetConfig = {
  enabled: false,
  restartOnChange: false,
};

export interface ResourceProps {
  name: string,
  path: string,
};

export const Resource = observer(function Resource(props: ProjectItemProps) {
  const { entry, project } = props;

  const resourceConfig: ResourceAssetConfig = ProjectState.project.getAssetConfig(entry.path, defaultResourceConfig);
  const resourceStatus = StatusState.getResourceStatus(entry.path);
  const conflictingWithSystemResource = project.manifest.systemResources.includes(entry.name as SystemResource);

  const options = React.useContext(ProjectExplorerItemContext);

  const { renderItemControls, contextMenuItems, requiredContextMenuItems, renderItemChildren } = useItem({
    ...props,
    childrenCollapsed: true
  });
  const { expanded, toggleExpanded } = useExpandablePath(entry.path);

  const [renamerOpen, openRenamer, closeRenamer] = useOpenFlag(false);

  const handleDelete = React.useCallback(() => {
    ProjectState.project.deleteEntryConfirmFirst(entry.path, `Delete resource "${entry.name}"?`, () => null);
  }, [entry]);

  const {
    resourceIsRunning,
    resourceIsEnabled,
    lifecycleContextMenuItems,
    commandsOutputOpen,
    closeCommandsOutput,
  } = useResourceLifecycle(entry, resourceConfig, resourceStatus, conflictingWithSystemResource);

  const relocateTargetContextMenu = useItemRelocateTargetContextMenu(entry);

  const itemContextMenuItems: ContextMenuItemsCollection = React.useMemo(() => [
    ...lifecycleContextMenuItems,
    ContextMenuItemSeparator,
    ...relocateTargetContextMenu,
    ContextMenuItemSeparator,
    {
      id: 'delete',
      icon: deleteIcon,
      text: 'Delete resource',
      disabled: options.disableAssetDelete,
      onClick: handleDelete,
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
  ], [
    lifecycleContextMenuItems,
    contextMenuItems,
    options,
    handleDelete,
    openRenamer,
    relocateTargetContextMenu,
    requiredContextMenuItems,
  ]);

  const children = renderItemChildren();

  const { isDragging, isDropping, dragRef, dropRef } = useItemDragAndDrop(entry, projectExplorerItemType.ASSET, [
    projectExplorerItemType.FILE,
    projectExplorerItemType.FOLDER,
    NativeTypes.FILE,
  ]);

  const rootClassName = classnames(itemsStyles.wrapper, {
    [itemsStyles.dropping]: isDropping,
    [itemsStyles.dragging]: isDragging,
  });

  const rootTitle = conflictingWithSystemResource
    ? `"${SYSTEM_RESOURCES_NAMES[entry.name as SystemResource]}" is enabled, this resource will be ignored. Either rename this resource or disable "${SYSTEM_RESOURCES_NAMES[entry.name as SystemResource]}" in Project Settings`
    : `${entry.name} • ${resourceIsEnabled ? 'Enabled' : 'Disabled'}${resourceIsRunning ? ' • Running' : ''}`;

  return (
    <div className={rootClassName} ref={dropRef}>
      <Title title={rootTitle}>
        {(ref) => (
          <ContextMenu
            ref={mergeRefs(dragRef, ref)}
            className={classnames(itemsStyles.item)}
            activeClassName={itemsStyles.itemActive}
            items={itemContextMenuItems}
            onClick={toggleExpanded}
          >
            <ItemState
              enabled={resourceIsEnabled}
              running={resourceIsRunning}
            />
            <ResourceIcon
              enabled={resourceIsEnabled}
              running={resourceIsRunning}
              conflictingWithSystemResource={conflictingWithSystemResource}
            />
            <div className={itemsStyles.itemTitle}>
              {entry.name}
            </div>
            <ResourceStatusNode
              resourceStatus={resourceStatus}
            />
          </ContextMenu>
        )}
      </Title>

      {expanded && (
        <div className={itemsStyles.children}>
          {renderItemControls()}
          <ProjectExplorerItemContextProvider options={contextOptions}>
            {children}
          </ProjectExplorerItemContextProvider>
        </div>
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

interface ResourceIconProps {
  enabled: boolean,
  running: boolean,
  conflictingWithSystemResource: boolean,
}
const ResourceIcon = React.memo(function ResourceIcon({ enabled, running, conflictingWithSystemResource }: ResourceIconProps) {
  if (conflictingWithSystemResource) {
    return (
      <span className={classnames(itemsStyles.itemIcon, s.conflicting)}>
        <BsExclamationCircle />
      </span>
    );
  }

  const icon = enabled
    ? enabledResourceIcon
    : resourceIcon;

  const iconsClassName = classnames(itemsStyles.itemIcon, {
    [s.running]: running,
  });

  return (
    <span className={iconsClassName}>
      {icon}
    </span>
  );
});

function useResourceLifecycle(
  entry: FilesystemEntry,
  resourceConfig: ResourceAssetConfig,
  resourceStatus: ResourceStatus,
  conflictingWithSystemResource: boolean,
) {
  const resourceName = entry.name;
  const resourcePath = entry.path;

  const [commandsOutputOpen, openCommandsOutput, closeCommandsOutput] = useOpenFlag(false);

  const serverIsNotUp = !ServerState.isUp;
  const resourceIsRunning = ServerState.isResourceRunning(resourceName);

  const isEnabled = !!resourceConfig?.enabled;
  const isAutorestartOnChangeEnabled = !!resourceConfig?.restartOnChange;

  const handleToggleEnabled = React.useCallback(() => {
    const request: APIRQ.ProjectSetAssetConfig<ResourceAssetConfig> = {
      assetPath: resourcePath,
      config: {
        enabled: !isEnabled,
      },
    };

    Api.send(projectApi.setAssetConfig, request);
  }, [resourcePath, isEnabled]);

  const handleToggleAutorestartEnabled = React.useCallback(() => {
    const request: APIRQ.ProjectSetAssetConfig<ResourceAssetConfig> = {
      assetPath: resourcePath,
      config: {
        restartOnChange: !isAutorestartOnChangeEnabled,
      },
    };

    Api.send(projectApi.setAssetConfig, request);
  }, [resourcePath, isAutorestartOnChangeEnabled]);

  const handleRestart = React.useCallback(() => {
    Api.send(serverApi.restartResource, resourceName);
  }, [resourceName]);
  const handleStop = React.useCallback(() => {
    Api.send(serverApi.stopResource, resourceName);
  }, [resourceName]);
  const handleStart = React.useCallback(() => {
    Api.send(serverApi.startResource, resourceName);
  }, [resourceName]);

  const lifecycleContextMenuItems: ContextMenuItemsCollection = React.useMemo(() => {
    const serverRelatedItems: ContextMenuItemsCollection = [];

    if (resourceIsRunning) {
      serverRelatedItems.push({
        id: 'stop',
        icon: stopIcon,
        text: 'Stop',
        disabled: serverIsNotUp || conflictingWithSystemResource,
        onClick: handleStop,
      }, {
        id: 'restart',
        icon: refreshIcon,
        text: 'Restart',
        disabled: serverIsNotUp || conflictingWithSystemResource,
        onClick: handleRestart,
      });
    } else {
      serverRelatedItems.push({
        id: 'Start',
        icon: startIcon,
        text: 'Start',
        disabled: serverIsNotUp || conflictingWithSystemResource,
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
          ? enabledResourceIcon
          : disabledResourceIcon,
        text: isEnabled
          ? 'Disable resource'
          : 'Enable resource',
        disabled: conflictingWithSystemResource,
        onClick: handleToggleEnabled,
      },
      {
        id: 'toggle-autorestart-enabled',
        icon: isAutorestartOnChangeEnabled
          ? checkedIcon
          : uncheckedIcon,
        text: isAutorestartOnChangeEnabled
          ? 'Disable restart on change'
          : 'Enable restart on change',
        disabled: conflictingWithSystemResource,
        onClick: handleToggleAutorestartEnabled,
      },
      ...fxdkCommandsItems,
    ];
  }, [
    serverIsNotUp,
    resourceName,
    resourceStatus,
    resourceIsRunning,
    isEnabled,
    isAutorestartOnChangeEnabled,
    handleStop,
    handleRestart,
    handleStart,
    openCommandsOutput,
    conflictingWithSystemResource,
  ]);

  return {
    resourceIsEnabled: isEnabled,
    resourceIsRunning,
    lifecycleContextMenuItems,
    commandsOutputOpen,
    closeCommandsOutput,
  };
}
