import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import { projectNamePattern } from 'constants/patterns';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useApiMessage, useDebouncedCallback } from 'utils/hooks';
import { ProjectCreateRequest } from 'shared/api.requests';
import { ProjectCreateCheckResult } from 'shared/project.types';
import { PathSelector } from 'components/controls/PathSelector/PathSelector';
import { BsBoxArrowUpRight } from 'react-icons/bs';
import { observer } from 'mobx-react-lite';
import { ProjectState } from 'store/ProjectState';
import s from './ProjectCreator.module.scss';


function formatProjectPathHint(projectPath: string, projectName: string) {
  let hint = 'Select a folder in which project will be created';

  if (projectPath) {
    hint = 'Project path: ' + projectPath;

    if (projectName) {
      hint += '\\' + projectName;
    }
  }

  return hint;
}

export const ProjectCreator = observer(function ProjectCreator() {
  const [projectName, setProjectName] = React.useState('');
  const [projectPath, setProjectPath] = React.useState('');
  const [checkResult, setCheckResult] = React.useState<ProjectCreateCheckResult>({});

  // Whenever we see project open - close creator
  useApiMessage(projectApi.open, ProjectState.closeCreator);

  useApiMessage(projectApi.checkCreateResult, (results) => {
    setCheckResult(results);
  }, [setCheckResult]);

  const checkRequest = useDebouncedCallback((projectPath: string, projectName: string) => {
    if (!projectPath || !projectName) {
      return;
    }

    const request: ProjectCreateRequest = {
      projectPath,
      projectName,
    };

    sendApiMessage(projectApi.checkCreateRequest, request);
  }, 250);

  const handleCreateProject = React.useCallback(() => {
    if (projectPath) {
      const request: ProjectCreateRequest = {
        projectPath,
        projectName,
      };

      sendApiMessage(projectApi.create, request);
    }
  }, [projectName, projectPath]);

  const handleProjectPathChange = React.useCallback((newProjectPath: string) => {
    setProjectPath(newProjectPath);
    checkRequest(newProjectPath, projectName);
  }, [projectName, setProjectPath, checkRequest]);

  const handleProjectNameChange = React.useCallback((newProjectName: string) => {
    setProjectName(newProjectName);
    checkRequest(projectPath, newProjectName);
  }, [projectPath, setProjectName, checkRequest]);

  const hint = formatProjectPathHint(projectPath, projectName);
  const canCreate = projectPath && projectName;

  return (
    <Modal fullWidth onClose={ProjectState.closeCreator}>
      <div className={s.root}>
        <h3 className="modal-header">
          Create New Project
        </h3>

        <div className={s['name-input']}>
          <Input
            autofocus
            className={s.input}
            label="Project name"
            placeholder="Definitely not an RP project name"
            value={projectName}
            onChange={handleProjectNameChange}
            pattern={projectNamePattern}
          />
        </div>

        <div className={s['explorer-hint']}>
          {hint}
        </div>
        <div className="modal-block">
          <PathSelector
            value={projectPath}
            onChange={handleProjectPathChange}
            placeholder="Project path"
            dialogTitle="Select folder in which project will be created..."
            buttonIcon={<BsBoxArrowUpRight />}
          />
        </div>

        <div className="modal-actions">
          <Button
            theme="primary"
            text={checkResult.openProject ? 'Open' : 'Create'}
            disabled={!canCreate}
            onClick={handleCreateProject}
          />

          <Button text="Cancel" onClick={ProjectState.closeCreator} />
        </div>
      </div>
    </Modal>
  );
});
