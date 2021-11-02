import React from 'react';
import { BiWorld } from "react-icons/bi";
import { BsCardChecklist, BsMarkdown } from "react-icons/bs";
import { SiCsharp, SiCss3, SiHtml5, SiJavascript, SiJson, SiLua, SiTypescript, SiVuedotjs, SiEditorconfig, SiVisualstudio, SiWebpack, SiEslint, SiSass } from "react-icons/si";
import { fileNamePattern } from 'constants/patterns';
import { ProjectExplorerFileSystemItem } from '../explorer.fileSystemItem';
import { inlineExplorerItemCreator } from '../explorer.itemCreate';
import { fileIcon } from 'fxdk/ui/icons';
import { projectExplorerItemType } from '../explorer.dnd';
import { IFsEntry } from 'fxdk/project/common/project.types';
import { Project } from 'fxdk/project/browser/state/project';
import { ProjectCommands } from '../../../browser/project.commands';
import { VscDiffIgnored } from 'react-icons/vsc';
import { FaFont, FaYarn } from 'react-icons/fa';

export const FileExplorerItemCreator = inlineExplorerItemCreator({
  id: 'file',
  icon: getIcon,
  label: 'File',
  order: 1,
  nameValidator: fileNamePattern,
  createCommandId: ProjectCommands.CREATE_FILE,
});

export class FileExplorerItemHandler extends ProjectExplorerFileSystemItem {
  constructor(entry: IFsEntry, entryPath: string) {
    super({
      entry,
      entryPath,
      rename: {
        validator: fileNamePattern,
      },
      options: {
        notAnAsset: true,
        noCreateChild: true,
        noFileSystemChildren: true,
      },
      dragAndDrop: {
        drag: {
          getItem: () => ({
            type: projectExplorerItemType.FILE,
            entryPath: this.entryPath,
          }),
        },
      },
    });
  }

  getDefaultIcon() {
    return getIcon(this.entry.name);
  }

  getIcon() {
    return getIcon(this.entry.name);
  }

  getLabel() {
    return this.entry.name;
  }

  readonly handleClick = () => {
    Project.openFile(this.entryPath);
  };

  readonly handleDoubleClick = () => {
    Project.openFile(this.entryPath, true);
  };
}

function getIcon(name?: string): React.ReactNode {
  if (!name) {
    return fileIcon;
  }

  switch (name) {
    case '__resource.lua':
    case 'fxmanifest.lua': return <BsCardChecklist />;

    case 'yarn.lock': return <FaYarn />;

    case 'webpack.config.js': return <SiWebpack />;

    case '.eslintrc.js': return <SiEslint />;
  }

  const indexOfExtDot = name.lastIndexOf('.');
  let ext = 'generic';

  if (indexOfExtDot > -1) {
    ext = name.substr(indexOfExtDot + 1);
  }

  switch (ext) {
    case 'js': return <SiJavascript />;
    case 'lua': return <SiLua />;
    case 'json': return <SiJson />;
    case 'ts': return <SiTypescript />;
    case 'fxworld': return <BiWorld />;
    case 'cs': return <SiCsharp />;
    case 'html': return <SiHtml5 />;

    case 'css': return <SiCss3 />;
    case 'sass':
    case 'scss': return <SiSass />;

    case 'editorconfig': return <SiEditorconfig />;

    case 'gitignore':
    case 'fxdkignore': return <VscDiffIgnored />;

    case 'woff':
    case 'eot':
    case 'ttf':
    case 'woff2': return <FaFont />;

    case 'vue': return <SiVuedotjs />;

    case 'md': return <BsMarkdown />;

    case 'sln':
    case 'csproj': return <SiVisualstudio />;

    case 'generic':
    default: {
      return fileIcon;
    }
  }
}
