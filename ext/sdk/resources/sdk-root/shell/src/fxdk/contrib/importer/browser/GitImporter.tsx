import React from 'react';
import classnames from 'classnames';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { githubApi } from 'shared/api.events';
import { ImporterProps } from './Importer.types';
import { defaultAssetImportPathSelectableFilter, inferAssetName } from './Importer.utils';
import { APIRQ } from 'shared/api.requests';
import { useSendApiMessageCallback } from 'utils/hooks';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { Release } from './Release';
import { observer } from 'mobx-react-lite';
import { Api } from 'fxdk/browser/Api';
import { GitImporterApi } from '../common/importer.git';
import { FsBrowser } from 'fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser';
import { FsBrowserUtils } from 'fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser.utils';
import { Project } from 'fxdk/project/browser/state/project';
import s from './Importer.module.scss';
import { ModalActions, ModalContent } from 'fxdk/ui/Modal/Modal';

export const GitImporter = observer(function GitImporter({ onClose }: ImporterProps) {
  const [repository, setRepository] = React.useState('');
  const [assetName, setAssetName] = React.useState('');
  const [assetBasePath, setAssetBasePath] = React.useState(Project.path);

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
      const request: GitImporterApi.ImportGithubReleaseRequest = {
        name: assetName,
        basePath: assetBasePath,
        releaseUrl: release.downloadUrl,
      };

      Api.send(GitImporterApi.Endpoints.importRelease, request);
    } else {
      const request: GitImporterApi.ImportRepositoryRequest = {
        name: assetName,
        basePath: assetBasePath,
        repositoryUrl: repository,
      };

      Api.send(GitImporterApi.Endpoints.importRepository, request);
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

  const assetPathHintHint = assetBasePath === Project.path
    ? 'Location: project root'
    : `Location: ${FsBrowserUtils.getRelativePath(Project.path, assetBasePath)}`;

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
      <ModalContent>
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

        <div className="modal-label">
          {assetPathHintHint}
        </div>

        <FsBrowser
          className={classnames(s.explorer, 'modal-block')}
          selectedPath={assetBasePath}
          onSelectPath={setAssetBasePath}
          selectableFilter={defaultAssetImportPathSelectableFilter}
        />

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
            <div className={s.releases}>
              {releasesNodes}
            </div>
          </>
        )}
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
