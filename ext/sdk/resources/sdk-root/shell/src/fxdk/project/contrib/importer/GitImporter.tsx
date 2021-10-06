import * as React from 'react';
import classnames from 'classnames';
import { GitAssetImportRequest } from 'backend/project/asset/importer-contributions/git-importer/git-importer.types';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Explorer } from 'fxdk/ui/Explorer/Explorer';
import { combineVisibilityFilters, visibilityFilters } from 'fxdk/ui/Explorer/Explorer.filters';
import { assetApi, githubApi } from 'shared/api.events';
import { FilesystemEntry } from 'shared/api.types';
import { assetImporterTypes } from 'shared/asset.types';
import { ImporterProps } from './Importer.types';
import { getRelativePath } from 'fxdk/ui/Explorer/Explorer.utils';
import { inferAssetName } from './Importer.utils';
import { APIRQ } from 'shared/api.requests';
import { useSendApiMessageCallback } from 'utils/hooks';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { ReleaseAssetImportRequest } from 'backend/project/asset/importer-contributions/release-importer/release-importer.types';
import { Release } from './Release';
import { ScrollContainer } from 'fxdk/ui/ScrollContainer/ScrollContainer';
import { ProjectState } from 'store/ProjectState';
import { observer } from 'mobx-react-lite';
import { Api } from 'fxdk/browser/Api';
import s from './Importer.module.scss';

const resourceFolderSelectableFilter = (entry: FilesystemEntry) => {
  return entry.isDirectory && !entry.meta.isResource;
};
const resourceFolderVisibilityFilter = combineVisibilityFilters(
  visibilityFilters.hideAssets,
  visibilityFilters.hideFiles,
  visibilityFilters.hideDotFilesAndDirs,
);

const noReleaseInfo: APIRQ.ReleaseInfo = {
  name: 'Import git repository as is',
  body: '',
  createdAt: '',
  downloadUrl: '',
};

export const GitImporter = observer(function GitImporter({ onClose }: ImporterProps) {
  const project = ProjectState.project;

  const [repository, setRepository] = React.useState('');
  const [assetName, setAssetName] = React.useState('');
  const [assetBasePath, setAssetBasePath] = React.useState(project.path);

  const inferredAssetNameRef = React.useRef('');

  const [releaseName, setReleaseName] = React.useState<string | null>(null);
  const [dontRelease, setDontRelease] = React.useState<boolean>(false);
  const [releases, setReleases] = React.useState<APIRQ.ReleaseInfo[]>([]);
  const [fetchingReleases, setFetchingReleases] = React.useState<boolean>(false);

  const doImport = React.useCallback(() => {
    if (!repository) {
      return;
    }

    const release = releaseName && releases.find(release => release.name === releaseName);

    if (release) {
      const request: ReleaseAssetImportRequest = {
        importerType: assetImporterTypes.release,
        assetName,
        assetBasePath,
        data: {
          releaseUrl: release.downloadUrl
        },
      };

      Api.send(assetApi.import, request);
    } else {
      const request: GitAssetImportRequest = {
        importerType: assetImporterTypes.git,
        assetName,
        assetBasePath,
        data: {
          repoUrl: repository
        },
      };

      Api.send(assetApi.import, request);
    }

    onClose();
  }, [repository, assetName, assetBasePath, onClose, releases, releaseName]);

  const fetchReleases = useSendApiMessageCallback<APIRQ.FetchReleases, APIRQ.FetchReleasesResponse>(githubApi.fetchReleases, (error, response) => {
    setFetchingReleases(false);
    setReleaseName(null);

    if (response.success) {
      setReleases(response.releases);
    } else {
      setReleases([]);
    }
  });

  const handleRepositoryChange = React.useCallback((nextRepository) => {
    setRepository(nextRepository);

    if (nextRepository) {
      // If inferred asset name did not change - infer more!
      if (inferredAssetNameRef.current === assetName) {
        const inferredAssetName = nextRepository
          ? inferAssetName(nextRepository)
          : '';

        inferredAssetNameRef.current = inferredAssetName;
        setAssetName(inferredAssetName);
      }

      const request: APIRQ.FetchReleases = {
        repoUrl: nextRepository,
      };

      setFetchingReleases(true);
      fetchReleases(request);
    } else {
      setReleaseName(null);
      setReleases([]);
    }
  }, [setRepository, setAssetName, assetName, fetchReleases]);

  const handleSetRelease = React.useCallback((releaseName: string) => {
    setReleaseName(releaseName);
    setDontRelease(false);
  }, [setReleaseName, setDontRelease]);

  const handleDontReleaseChange = React.useCallback((newDontRelease: boolean) => {
    setReleaseName(null);
    setDontRelease(newDontRelease);
  }, [setDontRelease, setReleaseName]);

  const assetPathHintHint = assetBasePath === project.path
    ? 'Location: project root'
    : `Location: ${getRelativePath(project.path, assetBasePath)}`;

  const canImport = repository && assetName && assetBasePath && (!releases?.length || releaseName);

  const releasesNodes = releases.map((release, index) => (
    <Release
      key={release.name + index}
      info={release}
      onClick={() => handleSetRelease(release.name)}
      checked={releaseName === release.name}
    />
  ));
  const hasReleases = !!releases.length;

  return (
    <>
      <div className="modal-label">
        Repository URL:
      </div>
      <div className="modal-block">
        <Input
          noSpellCheck
          showLoader={fetchingReleases}
          value={repository}
          onChange={handleRepositoryChange}
        />
      </div>

      <div className="modal-block">
        <Input
          value={assetName}
          onChange={setAssetName}
          label="Asset name:"
        />
      </div>

      {hasReleases && (
        <>
          <div className="modal-label">
            Select release:
          </div>
          <div className="modal-block">
            <Checkbox
              value={releaseName === null}
              label="Import repository as is"
              onChange={() => setReleaseName(null)}
            />
          </div>
          <ScrollContainer className={s.releases}>
            {releasesNodes}
          </ScrollContainer>
        </>
      )}

      <div className="modal-label">
        {assetPathHintHint}
      </div>
      <Explorer
        className={classnames(s.explorer, 'modal-block')}
        baseEntry={project.entry}
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
