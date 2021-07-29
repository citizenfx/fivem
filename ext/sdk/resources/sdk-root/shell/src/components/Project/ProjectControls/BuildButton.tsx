import * as React from 'react';
import { observer } from 'mobx-react-lite';
import { projectBuildIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';
import { Title } from 'components/controls/Title/Title';

export interface BuildButtonProps {
  className: string,
}

export const BuildButton = observer(function BuildButton({ className }: BuildButtonProps) {
  return (
    <Title animated={false} delay={0} fixedOn="bottom" title="Build project">
      {(ref) => (
        <button
          ref={ref}
          className={className}
          disabled={!ProjectState.hasProject}
          onClick={() => ProjectState.buildProject()}
          data-tour-id="project-build"
        >
          {projectBuildIcon}
        </button>
      )}
    </Title>
  );
});
