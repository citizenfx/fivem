import * as React from 'react';
import classnames from 'classnames';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Explorer } from 'components/Explorer/Explorer';
import { combineVisibilityFilters, visibilityFilters } from 'components/Explorer/Explorer.filters';
import { ProjectContext } from 'contexts/ProjectContext';
import { assetApi } from 'shared/api.events';
import { FilesystemEntry } from 'shared/api.types';
import { assetImporterTypes } from 'shared/asset.types';
import { sendApiMessage } from 'utils/api';
import { ImporterProps } from './Importer.types';
import { getRelativePath } from 'components/Explorer/Explorer.utils';
import { FsAssetImportRequest } from 'backend/project/asset/importer-contributions/fs-importer/fs-importer.types';
import { PathSelector } from 'components/controls/PathSelector/PathSelector';
import { inferAssetName } from './Importer.utils';
import s from './Importer.module.scss';

const resourceFolderSelectableFilter = (entry: FilesystemEntry) => {
  return entry.isDirectory && !entry.meta.isResource;
};
const resourceFolderVisibilityFilter = combineVisibilityFilters(
  visibilityFilters.hideAssets,
  visibilityFilters.hideFiles,
  visibilityFilters.hideDotFilesAndDirs,
);

export const FsImporter = React.memo(function FsImporter({ onClose }: ImporterProps) {
  const { project, projectEntry } = React.useContext(ProjectContext);

  const [sourcePath, setSourcePath] = React.useState('');
  const [assetName, setAssetName] = React.useState('');
  const [assetBasePath, setAssetBasePath] = React.useState('');

  const canImport = sourcePath && assetName && assetBasePath;

  const doImport = React.useCallback(() => {
    if (!canImport) {
      return;
    }

    const request: FsAssetImportRequest = {
      importerType: assetImporterTypes.fs,
      assetName,
      assetBasePath,
      data: {
        sourcePath,
      },
    };

    sendApiMessage(assetApi.import, request);
    onClose();
  }, [canImport, sourcePath, assetName, assetBasePath, onClose]);

  const handleSourcePathSelected = React.useCallback((nextSourcePath) => {
    setSourcePath(nextSourcePath);
    const inferredAssetName = inferAssetName(nextSourcePath);
    if (inferredAssetName && !assetName) {
      setAssetName(inferredAssetName);
    }
  }, [setSourcePath, setAssetName, assetName]);

  const assetBaseRelativePath = getRelativePath(project.path, assetBasePath);
  const assetPathHintHint = assetBasePath === project.path
    ? 'Location: project root'
    : `Location: ${assetBaseRelativePath}`;

  return (
    <>
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
      <Explorer
        className={classnames(s.explorer, 'modal-block')}
        baseEntry={projectEntry}
        pathsMap={project.fs}
        selectedPath={assetBasePath}
        onSelectPath={setAssetBasePath}
        selectableFilter={resourceFolderSelectableFilter}
        visibilityFilter={resourceFolderVisibilityFilter}
      />

      <div className="modal-actions">
        <Button
          theme="primary"
          text="Import"
          onClick={doImport}
          disabled={!canImport}
        />

        <Button
          text="Close"
          onClick={onClose}
        />
      </div>
    </>
  );
});
