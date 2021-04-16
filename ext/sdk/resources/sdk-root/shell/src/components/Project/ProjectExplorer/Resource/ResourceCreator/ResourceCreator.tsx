import React from 'react';
import { observer } from 'mobx-react-lite';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Explorer } from 'components/Explorer/Explorer';
import { Modal } from 'components/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { sendApiMessage } from 'utils/api';
import { assetApi } from 'shared/api.events';
import { FilesystemEntry } from 'shared/api.types';
import { AssetCreateRequest } from 'shared/api.requests';
import { combineVisibilityFilters, visibilityFilters } from 'components/Explorer/Explorer.filters';
import { getRelativePath } from 'components/Explorer/Explorer.utils';
import { ResourceTemplate } from './ResourceTemplate/ResourceTemplate';
import { resourceTemplateDescriptors } from 'resource-templates/descriptors-list';
import { assetTypes } from 'shared/asset.types';
import { ProjectState } from 'store/ProjectState';
import s from './ResourceCreator.module.scss';


const resourceFolderSelectableFilter = (entry: FilesystemEntry) => {
  return entry.isDirectory && !entry.meta.isResource;
};
const resourceFolderVisibilityFilter = combineVisibilityFilters(
  visibilityFilters.hideAssets,
  visibilityFilters.hideFiles,
  visibilityFilters.hideDotFilesAndDirs,
);

export const ResourceCreator = observer(function ResourceCreator() {
  const project = ProjectState.project;

  const [resourceName, setResourceName] = React.useState('');
  const [resourcePath, setResourcePath] = React.useState(ProjectState.resourceCreatorDir);
  const [resourceTemplateId, setResourceTemplateId] = React.useState(resourceTemplateDescriptors[0].id);

  // In case if path has been changed we should be acknowledged
  React.useEffect(() => {
    setResourcePath(ProjectState.resourceCreatorDir);
  }, [ProjectState.resourceCreatorDir, setResourcePath]);

  const handleCreateResource = React.useCallback(() => {
    if (resourceName && project) {
      const request: AssetCreateRequest = {
        assetType: assetTypes.resource,
        assetName: resourceName,
        assetPath: resourcePath,
        data: {
          resourceTemplateId,
        },
      };

      sendApiMessage(assetApi.create, request);

      ProjectState.closeResourceCreator();
    }
  }, [resourceName, project, resourcePath, resourceTemplateId]);

  const resourceRelativePath = getRelativePath(project.path, resourcePath);
  const resourcePathHint = resourcePath === project.path
    ? 'Location: project root'
    : `Location: ${resourceRelativePath}`;

  return (
    <Modal fullWidth onClose={ProjectState.closeResourceCreator}>
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
            onClick={ProjectState.closeResourceCreator}
          />
        </div>
      </div>
    </Modal>
  );
});
