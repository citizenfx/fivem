import React from 'react';
import { observer } from 'mobx-react-lite';
import { newResourceIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';

export interface NewResourceProps {
  className: string,
}

export const NewResource = observer(function NewResource({ className }: NewResourceProps) {
  const project = ProjectState.project;

  const handleClick = React.useCallback(() => {
    if (!project) {
      return;
    }

    ProjectState.openResourceCreator(project.path);
  }, [project.path]);

  return (
    <button
      className={className}
      onClick={handleClick}
      data-label="New resource"
      data-tour-id="new-resource"
    >
      {newResourceIcon}
    </button>
  );
});
