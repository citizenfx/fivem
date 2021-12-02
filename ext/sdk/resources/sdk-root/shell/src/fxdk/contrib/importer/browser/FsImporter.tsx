import React from 'react';
import classnames from 'classnames';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { ImporterProps } from './Importer.types';
import { PathSelector } from 'fxdk/ui/controls/PathSelector/PathSelector';
import { defaultAssetImportPathSelectableFilter, inferAssetName } from './Importer.utils';
import { observer } from 'mobx-react-lite';
import { Api } from 'fxdk/browser/Api';
import { FsImporterApi } from '../common/importer.fs';
import { FsBrowser } from 'fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser';
import { FsBrowserUtils } from 'fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser.utils';
import { Project } from 'fxdk/project/browser/state/project';
import s from './Importer.module.scss';
import { ModalActions, ModalContent } from 'fxdk/ui/Modal/Modal';

export const FsImporter = observer(function FsImporter({ onClose }: ImporterProps) {
  const [sourcePath, setSourcePath] = React.useState('');
  const [assetName, setAssetName] = React.useState('');
  const [assetBasePath, setAssetBasePath] = React.useState(Project.path);

  const canImport = sourcePath && assetName && assetBasePath;

  const doImport = React.useCallback(() => {
    if (!canImport) {
      return;
    }

    const request: FsImporterApi.ImportRequest = {
      name: assetName,
      basePath: assetBasePath,
      sourcePath,
    };

    Api.send(FsImporterApi.Endpoints.import, request);
    onClose();
  }, [canImport, sourcePath, assetName, assetBasePath, onClose]);

  const handleSourcePathSelected = React.useCallback((nextSourcePath) => {
    setSourcePath(nextSourcePath);
    const inferredAssetName = inferAssetName(nextSourcePath);
    if (inferredAssetName && !assetName) {
      setAssetName(inferredAssetName);
    }
  }, [setSourcePath, setAssetName, assetName]);

  const assetBaseRelativePath = FsBrowserUtils.getRelativePath(Project.path, assetBasePath);
  const assetPathHintHint = assetBasePath === Project.path
    ? 'Location: project root'
    : `Location: ${assetBaseRelativePath}`;

  return (
    <>
      <ModalContent>
        <div className="modal-label">
          Import from:
        </div>
        <PathSelector
          notOnlyFolders
          value={sourcePath}
          onChange={handleSourcePathSelected}
          className="modal-block"
        />

        <div className="modal-block">
          <Input
            value={assetName}
            onChange={setAssetName}
            label="Asset name:"
            description="If importing asset file, changing extension will change asset type, do not change it if unsure"
          />
        </div>

        <div className="modal-label">
          {assetPathHintHint}
        </div>
        <FsBrowser
          className={classnames(s.explorer, 'modal-block')}
          selectedPath={assetBasePath}
          onSelectPath={setAssetBasePath}
          selectableFilter={defaultAssetImportPathSelectableFilter}
        />
      </ModalContent>

      <ModalActions>
        <Button
          theme="primary"
          text="Import"
          onClick={doImport}
          disabled={!canImport}
        />
      </ModalActions>
    </>
  );
});
