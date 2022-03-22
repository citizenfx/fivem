import React from 'react';
import { observer } from 'mobx-react-lite';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { BsExclamationCircle } from 'react-icons/bs';
import { projectBuildingTaskName, ProjectBuildTaskStage } from 'shared/task.names';
import { useApiMessage } from 'utils/hooks';
import { Stepper } from 'fxdk/ui/controls/Stepper/Stepper';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { openInExplorerIcon, projectBuildIcon } from 'fxdk/ui/icons';
import { openInExplorerAndSelect } from 'utils/natives';
import { serverUpdateChannels } from 'shared/api.types';
import { ProjectBuildError } from 'shared/project.types';
import { TaskState } from 'store/TaskState';
import { ProjectBuilderError } from './ProjectBuilderError';
import { BuilderState } from './BuilderState';
import { Project } from 'fxdk/project/browser/state/project';
import { PathSelector } from 'fxdk/ui/controls/PathSelector/PathSelector';
import { ProjectApi } from 'fxdk/project/common/project.api';
import s from './ProjectBuilder.module.scss';

function getBuildPath(): string {
  if (Project.localStorage.buildPath) {
    return Project.localStorage.buildPath;
  }

  const [projectDirName, ...parts] = Project.path.split(/[\\/]/).reverse();

  return parts.reverse().join('\\') + '\\' + `${projectDirName}-build`;
}

const buildSteps: Record<ProjectBuildTaskStage, React.ReactNode> = {
  [ProjectBuildTaskStage.VerifyingBuildSite]: 'Verifying build site',
  [ProjectBuildTaskStage.RunningBuildCommands]: 'Running builds commands',
  [ProjectBuildTaskStage.PreparingBuildSite]: 'Preparing build site',
  [ProjectBuildTaskStage.DeployingToBuildSite]: 'Deploying to build site',
  [ProjectBuildTaskStage.Done]: 'Done',
};

export const ProjectBuilder = observer(function ProjectBuilder() {
  const buildTask = TaskState.get(projectBuildingTaskName);
  const buildInProgress = !!buildTask;

  const [buildError, setBuildError] = React.useState<ProjectBuildError | null>(null);
  useApiMessage(ProjectApi.BuilderEndpoints.error, (projectBuildError: ProjectBuildError) => {
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

    BuilderState.close();
  }, [buildTask]);

  const buildPath = getBuildPath();

  const openBuildPath = React.useCallback(() => {
    openInExplorerAndSelect(buildPath + '\\resources');
  }, [buildPath]);

  const buildPathDescription = buildPath
    ? (
      <>
        <a onClick={openBuildPath}>{buildPath}\resources {openInExplorerIcon}</a> directory will be created containing project resources.
      </>
    )
    : 'Select build folder';

  const handleBuildProject = React.useCallback(() => {
    setBuildTriggered(false);
    setBuildError(null);

    if (!Project.localStorage.buildPath && buildPath) {
      Project.localStorage.buildPath = buildPath;
    }

    Project.buildProject();
  }, [
    setBuildTriggered,
    setBuildError,
    buildPath,
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
          <PathSelector
            value={buildPath}
            onChange={(value) => Project.localStorage.buildPath = value}
            startPath={Project.path}
            dialogTitle="Select Project Build Folder..."
            disabled={buildInProgress}
            description={buildPathDescription}
          />
        </div>

        <div className="modal-label">
          Deploy options:
        </div>
        <div className="modal-block modal-combine">
          <Checkbox
            value={Project.localStorage.buildUseVersioning}
            onChange={(value) => Project.localStorage.buildUseVersioning = value}
            label="If possible, save previous build allowing build rollback"
          />
          <Checkbox
            value={Project.localStorage.buildUseArtifact}
            onChange={(value) => Project.localStorage.buildUseArtifact = value}
            label={`Include ${serverUpdateChannels[Project.manifest.serverUpdateChannel]} server artifact`}
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
