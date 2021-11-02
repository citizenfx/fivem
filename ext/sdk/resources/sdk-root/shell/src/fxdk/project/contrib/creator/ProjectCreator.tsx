import React from 'react';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { projectNamePattern } from 'constants/patterns';
import { useApiMessage, useDebouncedCallback } from 'utils/hooks';
import { APIRQ } from 'shared/api.requests';
import { ProjectCreateCheckResult } from 'shared/project.types';
import { PathSelector } from 'fxdk/ui/controls/PathSelector/PathSelector';
import { BsBoxArrowUpRight } from 'react-icons/bs';
import { observer } from 'mobx-react-lite';
import { Api } from 'fxdk/browser/Api';
import { ProjectCreatorState } from './ProjectCreatorState';
import s from './ProjectCreator.module.scss';
import { ProjectApi } from 'fxdk/project/common/project.api';


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

  useApiMessage(ProjectApi.LoaderEndpoints.checkCreateResult, (results) => {
    setCheckResult(results);
  }, [setCheckResult]);

  const checkRequest = useDebouncedCallback((projectPath: string, projectName: string) => {
    if (!projectPath || !projectName) {
      return;
    }

    const request: APIRQ.ProjectCreate = {
      projectPath,
      projectName,
    };

    Api.send(ProjectApi.LoaderEndpoints.checkCreateRequest, request);
  }, 250);

  const handleCreateProject = React.useCallback(() => {
    if (projectPath) {
      const request: APIRQ.ProjectCreate = {
        projectPath,
        projectName,
      };

      Api.send(ProjectApi.LoaderEndpoints.create, request);

      ProjectCreatorState.close();
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
    <Modal onClose={ProjectCreatorState.close}>
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

          <Button text="Cancel" onClick={ProjectCreatorState.close} />
        </div>
      </div>
    </Modal>
  );
});
