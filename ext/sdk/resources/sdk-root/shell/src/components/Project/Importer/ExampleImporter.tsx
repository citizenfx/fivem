import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { SystemResource } from 'backend/system-resources/system-resources-constants';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Explorer } from 'components/Explorer/Explorer';
import { combineVisibilityFilters, visibilityFilters } from 'components/Explorer/Explorer.filters';
import { getRelativePath } from 'components/Explorer/Explorer.utils';
import { FilesystemEntry } from 'shared/api.types';
import { ProjectState } from 'store/ProjectState';
import s from './Importer.module.scss';
import { BsExclamationCircle } from 'react-icons/bs';
import { ExampleAssetImportRequest } from 'backend/project/asset/importer-contributions/example-importer/example-importer.types';
import { assetImporterTypes } from 'shared/asset.types';
import { sendApiMessage } from 'utils/api';
import { assetApi, projectApi } from 'shared/api.events';

const examples = {
  [SystemResource.MONEY]: {
    name: 'Money',
    description: 'An example money system using key-value storage (KVS)',
  },
  [SystemResource.MONEY_FOUNTAIN]: {
    name: 'Money fountain',
    description: (
      <>
        An example money system client containing a money fountain
        <br />
        Make sure to adjust dependencies file of <kbd>fxmanifest.lua</kbd> if you have different name for money example
        <br />
        <br />
        <em>Dependencies:</em> money example and map manager (check Project Settings if it is enabled)
      </>
    ),
  },
  [SystemResource.MONEY_FOUNTAIN_EXAMPLE_MAP]: {
    name: 'Money fountain example map',
    description: (
      <>
        An example money system fountain spawn point
        <br />
        <br />
        <em>Dependencies:</em> money fountain example
      </>
    ),
  },
  [SystemResource.PED_MONEY_DROPS]: {
    name: 'Ped money drops',
    description: 'An example money system client',
  },
  [SystemResource.EXAMPLE_LOADSCREEN]: {
    name: 'Loading screen',
    description: 'An example custom loading screen',
  },
};

const resourceFolderSelectableFilter = (entry: FilesystemEntry) => {
  return entry.isDirectory && !entry.meta.isResource;
};
const resourceFolderVisibilityFilter = combineVisibilityFilters(
  visibilityFilters.hideAssets,
  visibilityFilters.hideFiles,
  visibilityFilters.hideDotFilesAndDirs,
);

export const ExampleImporter = observer(function ExampleImporter({ onClose }: { onClose(): void }) {
  const { project } = ProjectState;

  const [example, setExample] = React.useState<SystemResource | void>();
  const [assetName, setAssetName] = React.useState('');
  const [assetBasePath, setAssetBasePath] = React.useState(project.path);

  const assetPathHintHint = assetBasePath === project.path
    ? 'Location: project root'
    : `Location: ${getRelativePath(project.path, assetBasePath)}`;

  const doImport = React.useCallback(() => {
    if (!example || !assetName || !assetBasePath) {
      return;
    }

    const request: ExampleAssetImportRequest = {
      importerType: assetImporterTypes.example,
      assetName,
      assetBasePath,
      data: {
        exampleName: example,
      },
    };

    sendApiMessage(assetApi.import, request);
    onClose();
  }, [example, assetName, assetBasePath, onClose]);

  const canImport = Boolean(example && assetName && assetBasePath);

  const examplesNodes = Object.entries(examples).map(([exampleName, descriptor]: [SystemResource, any]) => {
    return (
      <div
        key={exampleName}
        onClick={() => setExample(exampleName)}
        className={classnames(s['example-resource'], { [s.active]: exampleName === example })}
      >
        <div className={s.name}>
          {descriptor.name}
        </div>
        <div className={s.description}>
          {descriptor.description}
        </div>
      </div>
    );
  });

  return (
    <>
      <div className="modal-block">
        <Input
          autofocus
          value={assetName}
          onChange={setAssetName}
          label="Asset name:"
        />
      </div>

      <div className="modal-block">
        <div className="panel panel-info">
          <BsExclamationCircle />&nbsp;
          Some examples depend on other examples and/or system resources, make sure all dependencies are satisfied 😏
        </div>
      </div>

      <div className="modal-block">
        <div className={s.examples}>
          {examplesNodes}
        </div>
      </div>

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
