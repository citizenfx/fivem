import * as React from 'react';
import classnames from 'classnames';
import { GitAssetImportRequest } from 'backend/project/asset/importer-contributions/git-importer/git-importer.types';
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

export const GitImporter = React.memo(function GitImporter({ onClose }: ImporterProps) {
  const { project, projectEntry } = React.useContext(ProjectContext);

  const [repository, setRepository] = React.useState('');
  const [assetName, setAssetName] = React.useState('');
  const [assetBasePath, setAssetBasePath] = React.useState('');

  const doImport = React.useCallback(() => {
    if (!repository) {
      return;
    }

    const request: GitAssetImportRequest = {
      importerType: assetImporterTypes.git,
      assetName,
      assetBasePath,
      data: {
        repoUrl: repository,
      },
    };

    sendApiMessage(assetApi.import, request);
    onClose();
  }, [repository, assetName, assetBasePath, onClose]);

  const handleSetRepository = React.useCallback((nextRepository) => {
    setRepository(nextRepository);

    if (nextRepository && !assetName) {
      const inferredAssetName = inferAssetName(nextRepository);
      if (inferredAssetName) {
        setAssetName(inferredAssetName);
      }
    }
  }, [setRepository, setAssetName, assetName]);

  const assetBaseRelativePath = getRelativePath(project.path, assetBasePath);
  const assetPathHintHint = assetBasePath === project.path
    ? 'Location: project root'
    : `Location: ${assetBaseRelativePath}`;

  const canImport = repository && assetName && assetBasePath;

  return (
    <>
      <div className="modal-label">
        Repository url:
      </div>
      <div className="modal-block">
        <Input
          value={repository}
          onChange={handleSetRepository}
        />
      </div>

      <div className="modal-block">
        <Input
          value={assetName}
          onChange={setAssetName}
          label="Asset name:"
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
