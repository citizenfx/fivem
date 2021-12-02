import React from 'react';
import { ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import { Importer } from './Importer';
import { ShellCommands } from 'shell-api/commands';
import { ImporterCommads } from './importer.commands';
import { importAssetIcon } from 'fxdk/ui/icons';
import { ImporterState } from './ImporterState';

ShellCommands.register(ImporterCommads.OPEN, ImporterState.open);

ProjectParticipants.registerRender({
  id: 'importer-render',
  isVisible: () => ImporterState.isOpen,
  render: () => <Importer />,
});

ProjectParticipants.registerControl({
  id: 'import',
  icon: importAssetIcon,
  label: 'Import Asset',
  commandId: ImporterCommads.OPEN,
  introId: 'create-project-item',
});
