import React from 'react';
import { fxworldIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';
import { Title } from 'components/controls/Title/Title';

export interface NewMapProps {
  className: string,
}

export const NewMap = React.memo(function NewMap({ className }: NewMapProps) {
  return (
    <Title animated={false} delay={0} title="New map" fixedOn="right">
      {(ref) => (
        <button
          ref={ref}
          className={className}
          onClick={ProjectState.mapCreatorUI.open}
        >
          {fxworldIcon}
        </button>
      )}
    </Title>
  );
});
