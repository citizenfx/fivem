import React from 'react';
import { importAssetIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';

export interface ImportAssetProps {
  className: string,
}

export const ImportAsset = React.memo(function ImportAsset({ className }: ImportAssetProps) {
  return (
    <button
      className={className}
      onClick={ProjectState.openImporter}
      data-label="Import asset"
    >
      {importAssetIcon}
    </button>
  );
});
