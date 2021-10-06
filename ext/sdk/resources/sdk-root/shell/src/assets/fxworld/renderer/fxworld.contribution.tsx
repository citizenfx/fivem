import React from 'react';
import { IProjectRenderParticipant, ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import { ShellCommands } from 'shell-api/commands';
import { OpenFlag } from 'store/generic/OpenFlag';
import { FXWorldCreator } from './FXWorldCreator/FXWorldCreator';
import { FXWorldCommands } from './fxworld.commands';
import { StatusState } from 'store/StatusState';
import { Feature } from 'shared/api.types';
import { assetTypes } from 'shared/asset.types';
import { fxworldIcon } from 'constants/icons';

function isAvailable() {
  return !!StatusState.getFeature(Feature.worldEditor);
}

const CreatorState = new OpenFlag(false);

ShellCommands.register(FXWorldCommands.OPEN_CREATOR, CreatorState.open);

ProjectParticipants.registerRender({
  id: 'fxworld-asset-render',
  isVisible: () => isAvailable() && CreatorState.isOpen,
  render: () => <FXWorldCreator close={CreatorState.close} />,
});

ProjectParticipants.registerItemCreator({
  id: assetTypes.fxworld,
  icon: fxworldIcon,
  label: 'New map',
  commandId: FXWorldCommands.OPEN_CREATOR,
  enabled: isAvailable,
});
