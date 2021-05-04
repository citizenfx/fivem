import React from 'react';
import { newDirectoryIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';

export interface NewDirectoryProps {
  className: string,
}

export const NewDirectory = React.memo(function NewDirectory({ className }: NewDirectoryProps) {
  return (
    <button
      className={className}
      onClick={ProjectState.openDirectoryCreator}
      title="New Directory"
    >
      {newDirectoryIcon}
    </button>
  );
});
