import * as React from 'react';
import { observer } from 'mobx-react-lite';
import { projectBuildIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';

export interface BuildButtonProps {
  className: string,
}

export const BuildButton = observer(function BuildButton({ className }: BuildButtonProps) {
  return (
    <button
      className={className}
      disabled={!ProjectState.hasProject}
      onClick={() => ProjectState.buildProject()}
      data-label="Build project"
      data-tour-id="project-build"
    >
      {projectBuildIcon}
    </button>
  );
});
