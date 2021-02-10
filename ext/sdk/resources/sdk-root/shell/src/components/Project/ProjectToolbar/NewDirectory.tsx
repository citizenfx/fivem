import * as React from 'react';
import { newDirectoryIcon } from 'constants/icons';
import { ProjectContext } from 'contexts/ProjectContext';

export interface NewDirectoryProps {
  className: string,
}

export const NewDirectory = React.memo(function NewDirectory({ className }: NewDirectoryProps) {
  const { openDirectoryCreator } = React.useContext(ProjectContext);

  return (
    <button
      className={className}
      onClick={openDirectoryCreator}
      title="New Directory"
    >
      {newDirectoryIcon}
    </button>
  );
});
