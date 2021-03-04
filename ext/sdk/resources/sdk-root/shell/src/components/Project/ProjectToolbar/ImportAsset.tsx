import { ProjectContext } from 'contexts/ProjectContext';
import * as React from 'react';
import { BsBoxArrowInDownRight } from 'react-icons/bs';

export interface ImportAssetProps {
  className: string,
}

export const ImportAsset = React.memo(function ImportAsset({ className }: ImportAssetProps) {
  const { openImporter } = React.useContext(ProjectContext);

  return (
    <button
      title="Import asset"
      className={className}
      onClick={openImporter}
    >
      <BsBoxArrowInDownRight />
    </button>
  );
});
