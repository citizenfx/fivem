import * as React from 'react';
import { observer } from 'mobx-react-lite';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import { BsBoxArrowUpRight, BsExclamationCircle } from 'react-icons/bs';
import { projectBuildingTaskName, ProjectBuildTaskStage } from 'shared/task.names';
import { useApiMessage, useOpenFolderSelectDialog, UseOpenFolderSelectDialogOptions } from 'utils/hooks';
import {
  useProjectBuildPathVar,
  useProjectDeployArtifactVar,
  useProjectSteamWebApiKeyVar,
  useProjectTebexSecretVar,
  useProjectUseVersioningVar,
} from 'utils/projectStorage';
import { Stepper } from 'components/controls/Stepper/Stepper';
import { Checkbox } from 'components/controls/Checkbox/Checkbox';
import { openInExplorerIcon, projectBuildIcon } from 'constants/icons';
import { openInExplorerAndSelect } from 'utils/natives';
import { serverUpdateChannels } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { ProjectBuildError } from 'shared/project.types';
import { TaskState } from 'store/TaskState';
import { ProjectBuilderError } from './ProjectBuilderError';
import { ProjectState } from 'store/ProjectState';
import s from './ProjectBuilder.module.scss';

const buildSteps: Record<ProjectBuildTaskStage, React.ReactNode> = {
  [ProjectBuildTaskStage.VerifyingBuildSite]: 'Verifying build site',
  [ProjectBuildTaskStage.StoppingWatchCommands]: 'Stopping watch commands',
  [ProjectBuildTaskStage.RunningBuildCommands]: 'Running builds commands',
  [ProjectBuildTaskStage.PreparingBuildSite]: 'Preparing build site',
  [ProjectBuildTaskStage.DeployingToBuildSite]: 'Deploying to build site',
  [ProjectBuildTaskStage.Done]: 'Done',
};

export const ProjectBuilder = observer(function ProjectBuilder() {
  const project = ProjectState.project;

  const buildTask = TaskState.get(projectBuildingTaskName);
  const buildInProgress = !!buildTask;

  const [buildError, setBuildError] = React.useState<ProjectBuildError | null>(null);
  useApiMessage(projectApi.buildError, (projectBuildError: ProjectBuildError) => {
    setBuildError(projectBuildError);
  }, [setBuildError]);

  const [buildTriggered, setBuildTriggered] = React.useState(false);
  const buildStage = buildTask
    ? buildTask.stage
    : (buildTriggered ? 9999 : -1);

  React.useEffect(() => {
    if (buildTask && !buildTriggered) {
      setBuildTriggered(true);
    }
  }, [buildTask, buildTriggered, setBuildTriggered]);

  const handleModalClose = React.useCallback(() => {
    if (buildTask) {
      return;
    }

    ProjectState.closeBuilder();
  }, [buildTask]);

  const [useVersioning, setUseVersioning] = useProjectUseVersioningVar(project);
  const [deployArtifact, setDeployArtifact] = useProjectDeployArtifactVar(project);
  const [steamWebApiKey, setSteamWebApiKey] = useProjectSteamWebApiKeyVar(project);
  const [tebexSecret, setTebexSecret] = useProjectTebexSecretVar(project);

  const [buildPath, setBuildPath] = useProjectBuildPathVar(project);

  // This is so we actually save default buildPath to localstorage allowing for fast-project-build next time w/o showing this modal
  React.useEffect(() => {
    setBuildPath(buildPath);
  }, []);

  const folderSelectOptions: UseOpenFolderSelectDialogOptions = React.useMemo(() => ({
    startPath: project.path,
    dialogTitle: 'Select Project Build Folder...',
  }), [project.path]);
  const openFolderSelectDialog = useOpenFolderSelectDialog(folderSelectOptions, (folderPath) => {
    if (folderPath) {
      setBuildPath(folderPath);
    }
  });

  const openBuildPath = React.useCallback(() => {
    openInExplorerAndSelect(buildPath + '\\resources');
  }, [buildPath]);

  const buildPathDescription = buildPath
    ? (
      <>
        {buildPath}\resources directory will be created containing project resources.
        &nbsp;<a onClick={openBuildPath}>Open Build Path {openInExplorerIcon}</a>
      </>
    )
    : 'Select build folder';

  const handleBuildProject = React.useCallback(() => {
    setBuildTriggered(false);
    setBuildError(null);

    ProjectState.buildProject({
      useVersioning,
      deployArtifact,
      steamWebApiKey,
      tebexSecret,
    });
  }, [
    setBuildTriggered,
    setBuildError,
    useVersioning,
    deployArtifact,
    steamWebApiKey,
    tebexSecret,
  ]);

  const buildAllowed = !buildInProgress && !!buildPath;

  return (
    <Modal fullWidth onClose={handleModalClose}>
      <div className={s.root}>
        <div className="modal-header">
          Project Builder
        </div>

        <div className="modal-label">
          Build directory:
        </div>
        <div className="modal-block">
          <div className={s['folder-select']}>
            <Input
              noSpellCheck
              className={s.input}
              value={buildPath}
              onChange={setBuildPath}
              placeholder="Select build directory"
              disabled={buildInProgress}
              description={buildPathDescription}
            />

            <Button
              className={s.button}
              text="Select"
              icon={<BsBoxArrowUpRight />}
              disabled={buildInProgress}
              onClick={openFolderSelectDialog}
            />
          </div>
        </div>

        <div className="modal-label">
          Deploy options:
        </div>
        <div className="modal-block modal-combine">
          <Checkbox
            value={useVersioning}
            onChange={setUseVersioning}
            label="If possible, save previous build allowing build rollback"
          />
          <Checkbox
            value={deployArtifact}
            onChange={setDeployArtifact}
            label={`Include server ${serverUpdateChannels[project.manifest.serverUpdateChannel]} artifact`}
          />
        </div>

        <div className="modal-block modal-combine">
          <Input
            type="password"
            label="Steam API key:"
            value={steamWebApiKey}
            onChange={setSteamWebApiKey}
            description={<>If you want to use Steam authentication — <a href="https://steamcommunity.com/dev/apikey">get a key</a></>}
          />
          <Input
            type="password"
            label="Tebex secret:"
            value={tebexSecret}
            onChange={setTebexSecret}
            description={<a href="https://server.tebex.io/settings/servers">Get Tebex secret</a>}
          />
        </div>

        <div className="modal-block">
          <div className="panel panel-info">
            <BsExclamationCircle />&nbsp;
            Tip: limit what gets deployed for each resource with <kbd>.fxdkignore</kbd> file in resource root that follows <kbd>.gitignore</kbd> file syntax.
          </div>
        </div>

        {!buildError && (
          <>
            <div className="modal-label">
              Building steps:
            </div>
            <div className="modal-block">
              <Stepper
                step={buildStage}
                steps={buildSteps}
              />
            </div>
          </>
        )}

        {!!buildError && (
          <div className={s.error}>
            <div className="modal-label">
              Building error:
            </div>
            <div className="modal-block">
              <ProjectBuilderError error={buildError} />
            </div>
          </div>
        )}

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Build"
            icon={projectBuildIcon}
            disabled={!buildAllowed}
            onClick={handleBuildProject}
          />

          <Button
            text="Close"
            disabled={buildInProgress}
            onClick={handleModalClose}
          />
        </div>
      </div>
    </Modal>
  );
});
