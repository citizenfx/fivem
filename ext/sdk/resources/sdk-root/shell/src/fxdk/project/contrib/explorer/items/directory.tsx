import React from 'react';
import { BsFolder, BsFolderFill, BsPuzzle } from "react-icons/bs";
import { NativeTypes } from 'react-dnd-html5-backend';
import { copyItemIntoEntryFromDrop } from '../basics';
import { directoryNamePattern } from 'constants/patterns';
import { PathState } from 'fxdk/project/browser/state/primitives/PathState';
import { ProjectExplorerFileSystemItem } from '../explorer.fileSystemItem';
import { inlineExplorerItemCreator } from '../explorer.itemCreate';
import { closedDirectoryIcon } from 'fxdk/ui/icons';
import { ProjectCommands } from '../../../browser/project.commands';
import { projectExplorerItemType } from '../explorer.dnd';
import { IFsEntry } from 'fxdk/project/common/project.types';
import { Indicator } from 'fxdk/ui/Indicator/Indicator';
import { Project } from 'fxdk/project/browser/state/project';

export const DirectoryExplorerItemCreator = inlineExplorerItemCreator({
  id: 'directory',
  icon: closedDirectoryIcon,
  label: 'Directory',
  order: 2,
  nameValidator: directoryNamePattern,
  createCommandId: ProjectCommands.CREATE_DIRECTORY,
});

export class DirectoryExplorerItemHandler extends ProjectExplorerFileSystemItem {
  readonly pathState = new PathState(this.entryPath, false);

  constructor(entry: IFsEntry, entryPath: string) {
    super({
      entry,
      entryPath,
      rename: {
        validator: directoryNamePattern,
      },
      options: {
        notAnAsset: true,
      },
      dragAndDrop: {
        drag: {
          getItem: () => ({
            type: projectExplorerItemType.FOLDER,
            entryPath: this.entryPath,
          }),
        },
        drop: {
          acceptTypes: [
            projectExplorerItemType.FILE,
            projectExplorerItemType.FOLDER,
            projectExplorerItemType.ASSET,
            NativeTypes.FILE,
          ],
          handleDrop: (item: unknown) => {
            copyItemIntoEntryFromDrop(this.entryPath, item as any);
            this.pathState.expand();
          },
        },
      },
    });

    if (this.pathState.isExpanded && !entry.childrenScanned) {
      let rICDone = false;
      const rIC = requestIdleCallback(() => {
        rICDone = true;
        Project.shallowScanEntryChildren(this.entryPath);
      });

      this.toDispose.register(() => !rICDone && cancelIdleCallback(rIC));
    }
  }

  getDefaultIcon(expanded: boolean) {
    return expanded
      ? <BsFolder />
      : <BsFolderFill />;
  }

  getIcon() {
    if (this.entry.fxmeta) {
      return <BsPuzzle />;
    }

    return this.pathState.isExpanded
      ? (this.entry.childrenScanned ? <BsFolder /> : <Indicator />)
      : <BsFolderFill />;
  }

  getLabel() {
    return this.entry.name;
  }

  shouldRenderChildren() {
    return this.pathState.isExpanded;
  }

  readonly handleClick = () => {
    this.pathState.toggle();

    if (!this.entry.childrenScanned) {
      Project.shallowScanEntryChildren(this.entryPath);
    }
  };
}
