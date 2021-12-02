import React from 'react';
import { ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import { ShellCommands } from 'shell-api/commands';
import { OpenFlag } from 'store/generic/OpenFlag';
import { ResourceCommands } from './resource.commands';
import { ResourceCreator } from './ResourceCreator/ResourceCreator';
import { resourceIcon } from 'fxdk/ui/icons';
import { ProjectExplorerParticipants } from 'fxdk/project/contrib/explorer/projectExplorerExtensions';
import { RESOURCE_ENTRY_HANDLE } from '../common/resource.constants';
import { ResourceAssetConfig } from '../common/resource.types';
import { ServerState } from 'store/ServerState';
import { ResourceProjectExplorerItem } from './ResourceProjectExplorerItem';
import { customExplorerItemCreator } from 'fxdk/project/contrib/explorer/explorer.itemCreate';
import { PrimitiveValue } from 'store/generic/PrimitiveValue';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';
import { Project } from 'fxdk/project/browser/state/project';

const CreatorState = new OpenFlag(false);
const CreatorBasePath = new PrimitiveValue('');

ShellCommands.register(ResourceCommands.OPEN_CREATOR, (basePath = '') => {
  CreatorBasePath.set(basePath);
  CreatorState.open();
});

ShellCommands.register(ResourceCommands.ENABLE_RESTART_ON_CHANGE, (assetPath: string) => {
  Project.setAssetConfig(assetPath, {
    restartOnChange: true,
  } as ResourceAssetConfig);
});

ShellCommands.register(ResourceCommands.DISABLE_RESTART_ON_CHANGE, (assetPath: string) => {
  Project.setAssetConfig(assetPath, {
    restartOnChange: false,
  } as ResourceAssetConfig);
});

ShellCommands.register(ResourceCommands.STOP_RESOURCE, ServerState.stopResource);
ShellCommands.register(ResourceCommands.START_RESOURCE, ServerState.startResource);
ShellCommands.register(ResourceCommands.RESTART_RESOURCE, ServerState.restartResource);

ProjectParticipants.registerRender({
  id: 'resource-asset-render',
  render: () => <ResourceCreator close={CreatorState.close} basePath={CreatorBasePath.value} />,
  isVisible: () => ProjectLoader.hasProject && CreatorState.isOpen,
});

ProjectExplorerParticipants.registerItemCreator(customExplorerItemCreator({
  id: 'resource',
  icon: resourceIcon,
  label: 'Resource',
  openCommandId: ResourceCommands.OPEN_CREATOR,
  order: 4,
  visible: (ctx) => !ctx?.[ResourceProjectExplorerItem.IN_RESOURCE_CONTEXT_KEY],
}));

ProjectExplorerParticipants.registerHandler(RESOURCE_ENTRY_HANDLE, ResourceProjectExplorerItem);
