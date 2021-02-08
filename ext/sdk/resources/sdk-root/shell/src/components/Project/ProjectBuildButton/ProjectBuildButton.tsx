import * as React from 'react';
import { ProjectContext } from 'contexts/ProjectContext';
import { projectBuildIcon } from 'constants/icons';
import s from './ProjectBuildButton.module.scss';

export const ProjectBuildButton = React.memo(function ProjectBuildButton() {
  const { project, build } = React.useContext(ProjectContext);

  return (
    <button
      className={s.root}
      disabled={!project}
      onClick={build}
      title="Build project"
    >
      {projectBuildIcon}
    </button>
  );
});
