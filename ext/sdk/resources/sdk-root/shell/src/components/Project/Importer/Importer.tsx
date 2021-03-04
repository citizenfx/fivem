import * as React from 'react';
import { Modal } from 'components/Modal/Modal';
import { ProjectContext } from 'contexts/ProjectContext';
import { AssetImporterType, assetImporterTypes } from 'shared/asset.types';
import { GitImporter } from './GitImporter';
import { ImporterProps } from './Importer.types';
import { FsImporter } from './FsImporter';
import { TabItem, TabSelector } from 'components/controls/TabSelector/TabSelector';
import s from './Importer.module.scss';

const importerTypeOptions: TabItem[] = [
  {
    label: 'Import from Git repository',
    value: assetImporterTypes.git,
  },
  {
    label: 'Import from file system on this PC',
    value: assetImporterTypes.fs,
  },
];

const importerRenderers: Record<AssetImporterType, React.ComponentType<ImporterProps>> = {
  [assetImporterTypes.git]: GitImporter,
  [assetImporterTypes.fs]: FsImporter,
};

export const Importer = React.memo(function Importer() {
  const { closeImporter } = React.useContext(ProjectContext);

  const [importerType, setImporterType] = React.useState<AssetImporterType>(assetImporterTypes.git);

  const ImporterRenderer = importerRenderers[importerType];

  return (
    <Modal fullWidth onClose={closeImporter}>
      <div className={s.root}>
        <div className="modal-header">
          Import asset
        </div>

        <div className="modal-block">
          <TabSelector
            value={importerType}
            items={importerTypeOptions}
            onChange={setImporterType}
          />
        </div>

        <ImporterRenderer onClose={closeImporter} />
      </div>
    </Modal>
  );
});
