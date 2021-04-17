import React from 'react';
import { BsBoxArrowInDownRight } from 'react-icons/bs';
import { ProjectState } from 'store/ProjectState';

export interface ImportAssetProps {
  className: string,
}

export const ImportAsset = React.memo(function ImportAsset({ className }: ImportAssetProps) {
  return (
    <button
      title="Import asset"
      className={className}
      onClick={ProjectState.openImporter}
    >
      <BsBoxArrowInDownRight />
    </button>
  );
});
