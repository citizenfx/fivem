import React from 'react';
import { observer } from 'mobx-react-lite';
import { newResourceIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';
import { Title } from 'components/controls/Title/Title';

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
    <Title animated={false} delay={0} title="New resource" fixedOn="right">
      {(ref) => (
        <button
          ref={ref}
          className={className}
          onClick={handleClick}
          data-tour-id="new-resource"
        >
          {newResourceIcon}
        </button>
      )}
    </Title>
  );
});
