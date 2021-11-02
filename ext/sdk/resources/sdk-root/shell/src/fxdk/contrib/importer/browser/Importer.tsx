import React from 'react';
import { Modal, ModalActions, ModalHeader } from 'fxdk/ui/Modal/Modal';
import { GitImporter } from './GitImporter';
import { FsImporter } from './FsImporter';
import { ExampleImporter } from './ExampleImporter';
import { TabItem, TabSelector } from 'fxdk/ui/controls/TabSelector/TabSelector';
import { observer } from 'mobx-react-lite';
import { ImporterState } from './ImporterState';
import { SplitHorizontal } from 'fxdk/ui/Modal/SplitHorizontalLayout';
import { openResourceIcon } from 'fxdk/ui/icons';
import { FaGit } from 'react-icons/fa';
import { FcOpenedFolder } from 'react-icons/fc';
import { Button } from 'fxdk/ui/controls/Button/Button';
import s from './Importer.module.scss';

const importerTypeOptions: TabItem[] = [
  {
    icon: <FaGit />,
    label: 'Git repository',
    value: 'git',
  },
  {
    icon: <FcOpenedFolder />,
    label: 'This PC',
    value: 'fs',
  },
  {
    icon: openResourceIcon,
    label: 'Example resources',
    value: 'examples',
  },
];

const importerRenderers = {
  git: GitImporter,
  fs: FsImporter,
  examples: ExampleImporter,
};

export const Importer = observer(function Importer() {
  const [importerType, setImporterType] = React.useState<string>('git');

  const ImporterRenderer = importerRenderers[importerType];

  return (
    <Modal fullWidth fullHeight onClose={ImporterState.close}>
      <SplitHorizontal.Layout>
        <SplitHorizontal.Left>
          <ModalHeader>
            Import asset from
          </ModalHeader>

          <TabSelector
            vertical
            className={s.tabs}
            value={importerType}
            items={importerTypeOptions}
            onChange={setImporterType}
          />

          <ModalActions>
            <Button
              text="Close"
              onClick={ImporterState.close}
            />
          </ModalActions>
        </SplitHorizontal.Left>

        <SplitHorizontal.Right>
          <ImporterRenderer onClose={ImporterState.close} />
        </SplitHorizontal.Right>
      </SplitHorizontal.Layout>
    </Modal>
  );
});
