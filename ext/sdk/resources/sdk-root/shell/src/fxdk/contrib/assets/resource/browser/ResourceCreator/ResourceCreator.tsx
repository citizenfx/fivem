import React from 'react';
import { observer } from 'mobx-react-lite';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { ResourceTemplate } from './ResourceTemplate/ResourceTemplate';
import { resourceTemplateDescriptors } from 'resource-templates/descriptors-list';
import { Api } from 'fxdk/browser/Api';
import { ResourceApi } from '../../common/resource.api';
import { Project } from 'fxdk/project/browser/state/project';
import { FsBrowserUtils } from 'fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser.utils';
import { FsBrowser } from 'fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser';
import s from './ResourceCreator.module.scss';

const resourceFolderSelectableFilter = FsBrowserUtils.combineSelectableFilters(
  FsBrowserUtils.filters.discardAssets,
  FsBrowserUtils.filters.discardFiles,
  FsBrowserUtils.filters.discardDotFilesAndDirs,
);

export interface ResourceCreatorProps {
  close: () => void,
  basePath: string,
}

export const ResourceCreator = observer(function ResourceCreator({ close, basePath }: ResourceCreatorProps) {
  const [resourceName, setResourceName] = React.useState('');
  const [resourcePath, setResourcePath] = React.useState(basePath || Project.path);
  const [resourceTemplateId, setResourceTemplateId] = React.useState(resourceTemplateDescriptors[0].id);

  const handleCreateResource = React.useCallback(() => {
    if (resourceName) {
      const request: ResourceApi.CreateRequest = {
        name: resourceName,
        basePath: resourcePath,
        resourceTemplateId,
      };

      Api.send(ResourceApi.Endpoints.create, request);

      close();
    }
  }, [resourceName, resourcePath, resourceTemplateId, close]);

  const resourceRelativePath = FsBrowserUtils.getRelativePath(Project.path, resourcePath);
  const resourcePathHint = resourcePath === Project.path
    ? 'Location: project root'
    : `Location: ${resourceRelativePath}`;

  return (
    <Modal fullWidth onClose={close}>
      <div className={s.root}>
        <div className="modal-header">
          Create Resource
        </div>

        <Input
          autofocus
          label="Name"
          placeholder="kiwigrape-matchmaking"
          value={resourceName}
          pattern={resourceNamePattern}
          className={s['name-input']}
          onChange={setResourceName}
          onSubmit={handleCreateResource}
        />

        <div className="modal-label">
          Template
        </div>
        <div className={s.templates}>
          {resourceTemplateDescriptors.map((resourceTemplate) => (
            <ResourceTemplate
              key={resourceTemplate.id}
              descriptor={resourceTemplate}
              onClick={() => setResourceTemplateId(resourceTemplate.id)}
              checked={resourceTemplateId === resourceTemplate.id}
            />
          ))}
        </div>

        <div className="modal-label">
          {resourcePathHint}
        </div>
        <FsBrowser
          className={s.explorer}
          selectedPath={resourcePath}
          onSelectPath={setResourcePath}
          selectableFilter={resourceFolderSelectableFilter}
        />

        <div className="modal-actions">
          <Button
            text="Create"
            theme="primary"
            onClick={handleCreateResource}
            disabled={!resourceName}
          />
          <Button
            text="Cancel"
            onClick={close}
          />
        </div>
      </div>
    </Modal>
  );
});
