import React from 'react';
import { fxworldIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';

export interface NewMapProps {
  className: string,
}

export const NewMap = React.memo(function NewMap({ className }: NewMapProps) {
  return (
    <button
      className={className}
      onClick={ProjectState.openMapCreator}
      data-label="New map"
    >
      {fxworldIcon}
    </button>
  );
});
