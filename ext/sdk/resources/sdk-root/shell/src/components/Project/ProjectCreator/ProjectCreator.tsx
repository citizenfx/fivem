import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Checkbox } from 'components/controls/Checkbox/Checkbox';
import { Input } from 'components/controls/Input/Input';
import { RootsExplorer } from 'components/Explorer/Explorer';
import { Modal } from 'components/Modal/Modal';
import { projectNamePattern } from 'constants/patterns';
import { ProjectContext } from 'contexts/ProjectContext';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useApiMessage, useDebouncedCallback, useFeature } from 'utils/hooks';
import { Feature } from 'shared/api.types';
import s from './ProjectCreator.module.scss';
import { ProjectCreateRequest } from 'shared/api.requests';
import { ProjectCreateCheckResult } from 'shared/project.types';
import { PathSelector } from 'components/controls/PathSelector/PathSelector';
import { BsBoxArrowUpRight } from 'react-icons/bs';


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

export const ProjectCreator = React.memo(function ProjectCreator() {
  const { closeCreator } = React.useContext(ProjectContext);

  const [projectName, setProjectName] = React.useState('');
  const [projectPath, setProjectPath] = React.useState('');
  const [checkResult, setCheckResult] = React.useState<ProjectCreateCheckResult>({});

  // Whenever we see project open - close creator
  useApiMessage(projectApi.open, closeCreator);

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
    <Modal fullWidth onClose={closeCreator}>
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

          <Button text="Cancel" onClick={closeCreator} />
        </div>
      </div>
    </Modal>
  );
});
