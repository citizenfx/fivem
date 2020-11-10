import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Checkbox } from 'components/controls/Checkbox/Checkbox';
import { Input } from 'components/controls/Input/Input';
import { RootsExplorer } from 'components/Explorer/Explorer';
import { Modal } from 'components/Modal/Modal';
import { projectNamePattern } from 'constants/patterns';
import { ProjectContext } from 'contexts/ProjectContext';
import { errorsApi, projectApi } from 'sdkApi/events';
import { sendApiMessage } from 'utils/api';
import { useApiMessage } from 'utils/hooks';
import s from './ProjectCreator.module.scss';


export const ProjectCreator = React.memo(function ProjectCreator() {
  const { creatorOpen, closeCreator } = React.useContext(ProjectContext);

  const [name, setName] = React.useState('');
  const [error, setError] = React.useState('');
  const [projectPath, setProjectPath] = React.useState<string>();
  const [withServerData, setWithServerData] = React.useState(true);

  let hint = 'Select a folder in which project will be created';

  if (projectPath) {
    hint = 'Project path: ' + projectPath;

    if (name) {
      hint += '\\' + name;
    }
  }

  useApiMessage(projectApi.open, closeCreator);

  useApiMessage(errorsApi.projectCreateError, (error: string) => {
    setError(error);
  }, [setError]);

  const handleCreateProject = React.useCallback(() => {
    setError('');

    if (projectPath) {
      sendApiMessage(projectApi.create, { projectPath, name, withServerData });
    }
  }, [name, projectPath, withServerData, setError]);

  if (!creatorOpen) {
    return null;
  }

  return (
    <Modal fullWidth onClose={closeCreator}>
      <div className={s.root}>
        <h3 className="modal-header">
          Create New Project
        </h3>

        <div className={s['name-input']}>
          <Input
            className={s.input}
            label="Project name"
            placeholder="Definitely not an RP project name"
            value={name}
            onChange={setName}
            pattern={projectNamePattern}
          />
        </div>

        <Checkbox
          value={withServerData}
          onChange={setWithServerData}
          label="Add cfx-server-data?"
          className={s.checkbox}
        />

        <div className={s['explorer-hint']}>
          {hint}
        </div>
        <RootsExplorer
          hideFiles
          selectedPath={projectPath}
          onSelectPath={setProjectPath}
        />

        {!!error && (
          <div className="modal-error">
            {error}
          </div>
        )}

        <div className="modal-actions">
          <Button theme="primary" text="Create" disabled={!projectPath || !name} onClick={handleCreateProject} />
          <Button text="Cancel" onClick={closeCreator} />
        </div>
      </div>
    </Modal>
  );
});
