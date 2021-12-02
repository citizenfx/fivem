import React from 'react';
import { IToolbarTitleViewParticipant, IToolbarViewParticipant, ToolbarParticipants } from 'fxdk/contrib/toolbar/browser/toolbarExtensions';
import { ProjectView } from './ProjectView';
import { ShellState } from 'store/ShellState';
import { ProjectControls } from './ProjectControls/ProjectControls';
import { ShellCommands } from 'shell-api/commands';
import { ProjectCommands } from './project.commands';
import { Api } from 'fxdk/browser/Api';
import { APIRQ } from 'shared/api.requests';
import { EntryRelocateOperation } from 'fxdk/project/browser/state/parts/RelocationContext';
import { Project } from './state/project';
import { ProjectLoader } from './state/projectLoader';
import { ProjectApi } from '../common/project.api';
import { closeIcon } from 'fxdk/ui/icons';

ShellCommands.register(ProjectCommands.ASSET_SET_ENABLED, (assetPath: string, enabled: boolean) => {
  Project.setAssetConfig(assetPath, {
    enabled,
  });
});

ShellCommands.register(ProjectCommands.SET_COPY_RELOCATION_CONTEXT, (entryPath: string) => {
  Project.relocation.set(entryPath, EntryRelocateOperation.Copy);
});

ShellCommands.register(ProjectCommands.SET_MOVE_RELOCATION_CONTEXT, (entryPath: string) => {
  Project.relocation.set(entryPath, EntryRelocateOperation.Move);
});

ShellCommands.register(ProjectCommands.APPLY_RELOCATION, (entryPath: string) => {
  Project.relocation.apply(entryPath);
});

ShellCommands.register(ProjectCommands.DELETE_ENTRY, (entryPath: string) => {
  Project.deleteEntry(entryPath);
});

ShellCommands.register(ProjectCommands.DELETE_ENTRY_CONFIRM_FIRST, (entryPath: string, title: string, children: () => React.ReactNode) => {
  Project.deleteEntryConfirmFirst(entryPath, title, children);
});

ShellCommands.register(ProjectCommands.OPEN_IN_EXPLORER, (entryPath: string) => {
  invokeNative('openFolderAndSelectFile', entryPath);
});

ShellCommands.register(ProjectCommands.CREATE_FILE, async (filePath: string, fileName: string) => {
  const fullFilePath: string = await Api.sendPromise(ProjectApi.FsEndpoints.createFile, {
    filePath,
    fileName,
  } as APIRQ.ProjectCreateFile);

  Project.openFile(fullFilePath, true);
});

ShellCommands.register(ProjectCommands.CREATE_DIRECTORY, (directoryPath: string, directoryName: string) => {
  Api.send(ProjectApi.FsEndpoints.createDirectory, {
    directoryPath,
    directoryName,
  } as APIRQ.ProjectCreateDirectory);
});

ToolbarParticipants.registerView(new class ProjectViewParticipant implements IToolbarViewParticipant {
  readonly id = 'project-view';

  render() {
    return (
      <ProjectView />
    );
  }
});

ToolbarParticipants.registerTitleView(new class ProjectTitleViewParticipant implements IToolbarTitleViewParticipant {
  readonly id = 'project-title-view';

  isVisible() {
    return ShellState.isMainPersonality;
  }

  render() {
    return (
      <ProjectControls />
    );
  }
});

ToolbarParticipants.registerMenuItem({
  id: 'project-close',
  order: Number.MAX_SAFE_INTEGER,
  group: 'project-switch',
  item: {
    id: 'project-close',
    text: 'Close Project',
    icon: closeIcon,
    onClick: ProjectLoader.close,
  },
});
