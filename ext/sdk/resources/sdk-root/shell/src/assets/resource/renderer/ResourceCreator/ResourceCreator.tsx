import React from 'react';
import { observer } from 'mobx-react-lite';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Explorer } from 'fxdk/ui/Explorer/Explorer';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { assetApi } from 'shared/api.events';
import { FilesystemEntry } from 'shared/api.types';
import { APIRQ } from 'shared/api.requests';
import { combineVisibilityFilters, visibilityFilters } from 'fxdk/ui/Explorer/Explorer.filters';
import { getRelativePath } from 'fxdk/ui/Explorer/Explorer.utils';
import { ResourceTemplate } from './ResourceTemplate/ResourceTemplate';
import { resourceTemplateDescriptors } from 'resource-templates/descriptors-list';
import { assetTypes } from 'shared/asset.types';
import { ProjectState } from 'store/ProjectState';
import { Api } from 'fxdk/browser/Api';
import s from './ResourceCreator.module.scss';


const resourceFolderSelectableFilter = (entry: FilesystemEntry) => {
  return entry.isDirectory && !entry.meta.isResource;
};
const resourceFolderVisibilityFilter = combineVisibilityFilters(
  visibilityFilters.hideAssets,
  visibilityFilters.hideFiles,
  visibilityFilters.hideDotFilesAndDirs,
);

export interface ResourceCreatorProps {
  close: () => void,
}

export const ResourceCreator = observer(function ResourceCreator({ close }: ResourceCreatorProps) {
  const project = ProjectState.project;

  const [resourceName, setResourceName] = React.useState('');
  const [resourcePath, setResourcePath] = React.useState(ProjectState.resourceCreatorDir);
  const [resourceTemplateId, setResourceTemplateId] = React.useState(resourceTemplateDescriptors[0].id);

  // In case if path has been changed we should be acknowledged
  React.useEffect(() => {
    setResourcePath(ProjectState.resourceCreatorDir);
  }, [ProjectState.resourceCreatorDir, setResourcePath]);

  const handleCreateResource = React.useCallback(() => {
    if (resourceName) {
      const request: APIRQ.AssetCreate = {
        assetType: assetTypes.resource,
        assetName: resourceName,
        assetPath: resourcePath,
        data: {
          resourceTemplateId,
        },
      };

      Api.send(assetApi.create, request);

      close();
    }
  }, [resourceName, project, resourcePath, resourceTemplateId, close]);

  const resourceRelativePath = getRelativePath(project.path, resourcePath);
  const resourcePathHint = resourcePath === project.path
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
        <Explorer
          className={s.explorer}
          baseEntry={project.entry}
          pathsMap={project.fs}
          selectedPath={resourcePath}
          onSelectPath={setResourcePath}
          selectableFilter={resourceFolderSelectableFilter}
          visibilityFilter={resourceFolderVisibilityFilter}
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
