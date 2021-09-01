import React from 'react';
import { newDirectoryIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';
import { Title } from 'components/controls/Title/Title';

export interface NewDirectoryProps {
  className: string,
}

export const NewDirectory = React.memo(function NewDirectory({ className }: NewDirectoryProps) {
  return (
    <Title animated={false} delay={0} title="New directory" fixedOn="right">
      {(ref) => (
        <button
          ref={ref}
          className={className}
          onClick={ProjectState.directoryCreatorUI.open}
        >
          {newDirectoryIcon}
        </button>
      )}
    </Title>
  );
});
