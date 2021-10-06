import React from 'react';
import { ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import { ShellCommands } from 'shell-api/commands';
import { OpenFlag } from 'store/generic/OpenFlag';
import { ResourceCommands } from './resource.commands';
import { ResourceCreator } from './ResourceCreator/ResourceCreator';
import { enabledResourceIcon } from 'constants/icons';
import { assetTypes } from 'shared/asset.types';
import { ProjectState } from 'store/ProjectState';

const CreatorState = new OpenFlag(false);

ShellCommands.register(ResourceCommands.OPEN_CREATOR, CreatorState.open);

ProjectParticipants.registerRender({
  id: 'resource-asset-render',
  render: () => <ResourceCreator close={CreatorState.close} />,
  isVisible: () => ProjectState.hasProject && CreatorState.isOpen,
});

ProjectParticipants.registerItemCreator({
  id: assetTypes.resource,
  icon: enabledResourceIcon,
  label: 'New resource',
  commandId: ResourceCommands.OPEN_CREATOR,
});
