import * as React from 'react';
import { ProjectContext } from 'contexts/ProjectContext';
import { projectBuildIcon } from 'constants/icons';

export interface BuildButtonProps {
  className: string,
}

export const BuildButton = React.memo(function BuildButton({ className }: BuildButtonProps) {
  const { project, build } = React.useContext(ProjectContext);

  return (
    <button
      className={className}
      disabled={!project}
      onClick={() => build()}
      title="Build project"
    >
      {projectBuildIcon}
    </button>
  );
});
