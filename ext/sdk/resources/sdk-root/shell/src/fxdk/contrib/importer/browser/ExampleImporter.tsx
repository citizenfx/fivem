import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { SystemResource } from 'backend/system-resources/system-resources-constants';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { BsExclamationCircle } from 'react-icons/bs';
import { Api } from 'fxdk/browser/Api';
import { ExamplesImporterApi } from '../common/importer.examples';
import { FsBrowser } from 'fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser';
import { FsBrowserUtils } from 'fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser.utils';
import { Project } from 'fxdk/project/browser/state/project';
import s from './Importer.module.scss';
import { defaultAssetImportPathSelectableFilter } from './Importer.utils';
import { ModalActions, ModalContent } from 'fxdk/ui/Modal/Modal';

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

export const ExampleImporter = observer(function ExampleImporter({ onClose }: { onClose(): void }) {
  const [example, setExample] = React.useState<SystemResource | void>();
  const [assetName, setAssetName] = React.useState('');
  const [assetBasePath, setAssetBasePath] = React.useState(Project.path);

  const assetPathHintHint = assetBasePath === Project.path
    ? 'Location: project root'
    : `Location: ${FsBrowserUtils.getRelativePath(Project.path, assetBasePath)}`;

  const doImport = React.useCallback(() => {
    if (!example || !assetName || !assetBasePath) {
      return;
    }

    const request: ExamplesImporterApi.ImportRequest = {
      name: assetName,
      basePath: assetBasePath,
      exampleName: example,
    };

    Api.send(ExamplesImporterApi.Endpoints.import, request);
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
      <ModalContent>
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
