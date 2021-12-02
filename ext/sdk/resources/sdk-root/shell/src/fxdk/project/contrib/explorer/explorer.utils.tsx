import React from 'react';
import classnames from "classnames";
import { IProjectExplorer } from "./projectExplorerItem";
import { IFsEntry } from 'fxdk/project/common/project.types';

export function joinPath(...parts: string[]): string {
  return parts.join('\\');
}

export function renderExplorerItemIcon(icon: IProjectExplorer.ItemIcon, baseClass: string): React.ReactNode {
  if (typeof icon === 'object' && icon !== null && ('icon' in icon)) {
    return (
      <div className={classnames(baseClass, icon.className)}>
        {icon.icon}
      </div>
    );
  }

  return (
    <div className={baseClass}>
      {icon}
    </div>
  );
}

export function isEntryNotAnAsset(entry: IFsEntry): boolean {
  return entry.handle === 'directory' || entry.handle === 'file';
}
