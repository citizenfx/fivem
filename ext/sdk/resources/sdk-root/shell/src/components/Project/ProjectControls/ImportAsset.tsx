import React from 'react';
import { importAssetIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';
import { Title } from 'components/controls/Title/Title';

export interface ImportAssetProps {
  className: string,
}

export const ImportAsset = React.memo(function ImportAsset({ className }: ImportAssetProps) {
  return (
    <Title animated={false} delay={0} fixedOn="bottom" title="Import asset">
      {(ref) => (
        <button
          ref={ref}
          className={className}
          onClick={ProjectState.openImporter}
        >
          {importAssetIcon}
        </button>
      )}
    </Title>
  );
});
