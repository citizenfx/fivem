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
import { Feature, ProjectCreateCheckResult } from 'shared/api.types';
import s from './ProjectCreator.module.scss';
import { ProjectCreateRequest } from 'shared/api.requests';


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
  const [withServerData, setWithServerData] = React.useState(false);
  const [checkResult, setCheckResult] = React.useState<ProjectCreateCheckResult>({});

  const canInstallServerData = useFeature(Feature.systemGitClientAvailable);

  // Whenever we see project open - close creator
  useApiMessage(projectApi.open, closeCreator);

  useApiMessage(projectApi.checkCreateResult, (results) => {
    setCheckResult(results);
  }, [setCheckResult]);

  const checkRequest = useDebouncedCallback((projectPath: string, projectName: string, withServerData: boolean) => {
    if (!projectPath || !projectName) {
      return;
    }

    const request: ProjectCreateRequest = {
      projectPath,
      projectName,
      withServerData,
    };

    sendApiMessage(projectApi.checkCreateRequest, request);
  }, 250);

  const handleCreateProject = React.useCallback(() => {
    if (projectPath) {
      const request: ProjectCreateRequest = {
        projectPath,
        projectName,
        withServerData,
      };

      sendApiMessage(projectApi.create, request);
    }
  }, [projectName, projectPath, withServerData]);

  const handleProjectPathChange = React.useCallback((newProjectPath: string) => {
    setProjectPath(newProjectPath);
    checkRequest(newProjectPath, projectName, withServerData);
  }, [projectName, withServerData, setProjectPath, checkRequest]);

  const handleProjectNameChange = React.useCallback((newProjectName: string) => {
    setProjectName(newProjectName);
    checkRequest(projectPath, newProjectName, withServerData);
  }, [projectPath, withServerData, setProjectName, checkRequest]);

  const handleWithServerDataChange = React.useCallback((newWithServerData: boolean) => {
    setWithServerData(newWithServerData);
    checkRequest(projectPath, projectName, newWithServerData);
  }, [projectPath, projectName, setWithServerData, checkRequest]);

  const hint = formatProjectPathHint(projectPath, projectName);
  const canCreate = projectPath && projectName;

  const serverDataCheckboxLabel = !canInstallServerData
    ? `Can't install cfx-server-data automatically as we failed to find git client on this machine, we're working on resolving this issue, sorry for inconveniece :(`
    : (
      checkResult.ignoreCfxServerData
        ? `cfx-server-data is already there! Though we don't know if that is what you need`
        : 'Add cfx-server-data automatically?'
    );

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

        <Checkbox
          value={withServerData}
          onChange={handleWithServerDataChange}
          label={serverDataCheckboxLabel}
          className={s.checkbox}
          disabled={!canInstallServerData}
        />

        <div className={s['explorer-hint']}>
          {hint}
        </div>
        <RootsExplorer
          hideFiles
          selectedPath={projectPath}
          onSelectPath={handleProjectPathChange}
        />

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
