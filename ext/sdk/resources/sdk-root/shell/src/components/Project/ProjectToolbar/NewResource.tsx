import { newResourceIcon } from 'constants/icons';
import { ProjectContext } from 'contexts/ProjectContext';
import * as React from 'react';

export interface NewResourceProps {
  className: string,
}

export const NewResource = React.memo(function NewResource({ className }: NewResourceProps) {
  const { project, setResourceCreatorDir, openResourceCreator } = React.useContext(ProjectContext);

  const handleClick = React.useCallback(() => {
    if (!project) {
      return;
    }

    setResourceCreatorDir(project.path);
    openResourceCreator();
  }, [project?.path, setResourceCreatorDir, openResourceCreator]);

  return (
    <button
      className={className}
      onClick={handleClick}
      title="New Resource"
    >
      {newResourceIcon}
    </button>
  );
});
