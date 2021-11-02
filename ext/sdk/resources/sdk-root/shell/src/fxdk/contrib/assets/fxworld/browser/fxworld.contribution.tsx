import { ShellCommands } from 'shell-api/commands';
import { FXWorldCommands } from './fxworld.commands';
import { StatusState } from 'store/StatusState';
import { Feature } from 'shared/api.types';
import { fxworldIcon } from 'fxdk/ui/icons';
import { ProjectExplorerParticipants } from 'fxdk/project/contrib/explorer/projectExplorerExtensions';
import { fxworldRecompile, FXWORLD_ENTRY_HANDLE } from '../common/fxworld-constants';
import { FXWORLD_FILE_EXT } from '../common/fxworld-types';
import { Api } from 'fxdk/browser/Api';
import { resourceNamePattern } from 'constants/patterns';
import { renameFileSystemEntry } from 'fxdk/project/contrib/explorer/basics';
import { ProjectExplorerFileSystemItem } from 'fxdk/project/contrib/explorer/explorer.fileSystemItem';
import { inlineExplorerItemCreator } from 'fxdk/project/contrib/explorer/explorer.itemCreate';
import { WEState } from 'personalities/world-editor/store/WEState';
import { projectExplorerItemType } from 'fxdk/project/contrib/explorer/explorer.dnd';
import { IFsEntry } from 'fxdk/project/common/project.types';
import { FXWorldApi } from '../common/fxworld.api';

function isAvailable() {
  return !!StatusState.getFeature(Feature.worldEditor);
}

ShellCommands.register(FXWorldCommands.CREATE, (basePath: string, name: string) => {
  const request: FXWorldApi.CreateRequest = {
    name,
    basePath,
  };

  console.log('creating map', request);

  Api.send(FXWorldApi.Endpoints.create, request);
});

ShellCommands.register(FXWorldCommands.RECOMPILE, (scope: string) => Api.sendScoped(fxworldRecompile, scope));

ProjectExplorerParticipants.registerItemCreator(inlineExplorerItemCreator({
  id: 'fxworld',
  icon: fxworldIcon,
  label: 'Map',
  order: 3,
  placeholder: 'Map name',
  createCommandId: FXWorldCommands.CREATE,
  nameValidator: resourceNamePattern,
  visible: isAvailable,
}));

ProjectExplorerParticipants.registerHandler(FXWORLD_ENTRY_HANDLE, class FXWorldProjectExplorerItem extends ProjectExplorerFileSystemItem {
  constructor(entry: IFsEntry, entryPath: string) {
    super({
      entry,
      entryPath,
      menuItems: [
        {
          id: 'recompile',
          label: 'Recompile',
          commandId: FXWorldCommands.RECOMPILE,
          commandArgs: () => [this.entry.name],
        },
      ],
      rename: {
        validator: resourceNamePattern,
        handleRename: (newName: string) => renameFileSystemEntry(this.entryPath, newName + FXWORLD_FILE_EXT),
      },
      dragAndDrop: {
        drag: {
          getItem: () => ({
            type: projectExplorerItemType.FILE,
            entry: this.entry,
          }),
        },
      },
    });
  }

  getDefaultIcon() {
    return fxworldIcon;
  }

  getIcon() {
    return fxworldIcon;
  }

  getLabel() {
    return this.entry.name.replace(FXWORLD_FILE_EXT, '');
  }

  readonly handleClick = () => WEState.openMap(this.entry, this.entryPath);
});
