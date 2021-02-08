import * as React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import { ProjectContext } from 'contexts/ProjectContext';
import { BsBoxArrowUpRight, BsExclamationCircle } from 'react-icons/bs';
import { useTask } from 'contexts/TaskContext';
import { projectBuildingTaskName, ProjectBuildTaskStage } from 'shared/task.names';
import { useOpenFolderSelectDialog } from 'utils/hooks';
import { useProjectClientStorageItem } from 'utils/projectStorage';
import { Stepper } from 'components/controls/Stepper/Stepper';
import { Checkbox } from 'components/controls/Checkbox/Checkbox';
import s from './ProjectBuilder.module.scss';
import { openInExplorerIcon, projectBuildIcon } from 'constants/icons';
import { openInExplorerAndSelect } from 'utils/natives';

const buildSteps: Record<ProjectBuildTaskStage, React.ReactNode> = {
  [ProjectBuildTaskStage.VerifyingBuildSite]: 'Verifying build site',
  [ProjectBuildTaskStage.StoppingWatchCommands]: 'Stopping watch commands',
  [ProjectBuildTaskStage.RunningBuildCommands]: 'Running builds commands',
  [ProjectBuildTaskStage.PreparingBuildSite]: 'Preparing build site',
  [ProjectBuildTaskStage.DeployingToBuildSite]: 'Deploying to build site',
  [ProjectBuildTaskStage.Done]: 'Done',
};

export const ProjectBuilder = React.memo(function ProjectBuilder() {
  const { project, closeBuilder, build } = React.useContext(ProjectContext);
  const buildTask = useTask(projectBuildingTaskName);
  const buildInProgress = !!buildTask;

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

    closeBuilder();
  }, [buildTask, closeBuilder]);

  const [useVersioning, setUseVersioning] = useProjectClientStorageItem(project, 'useVersioning', true);

  const [buildPath, setBuildPath] = useProjectClientStorageItem(project, 'buildPath', '');
  const openFolderSelectDialog = useOpenFolderSelectDialog(project.path, 'Select Project Build Folder...', (folderPath) => {
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
        {buildPath}\\resources directory will be created containing project resources.
        &nbsp;<a onClick={openBuildPath}>Open Build Path {openInExplorerIcon}</a>
      </>
    )
    : 'Select build folder';

  const handleBuildProject = React.useCallback(() => {
    setBuildTriggered(false);

    build();
  }, [setBuildTriggered, build]);

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
          Use versions:
        </div>
        <div className="modal-block">
          <Checkbox
            value={useVersioning}
            onChange={setUseVersioning}
            label="If possible, save previous build allowing build rollback"
          />
        </div>

        <div className="modal-block">
          <div className="panel panel-info">
            <BsExclamationCircle />&nbsp;
            Tip: limit what gets deployed for each resource with <kbd>.fxdkignore</kbd> file in resource root that follows <kbd>.gitignore</kbd> file syntax.
          </div>
        </div>

        <div className="modal-label">
          Building steps:
        </div>
        <div className="modal-block">
          <Stepper
            step={buildStage}
            steps={buildSteps}
          />
        </div>

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
