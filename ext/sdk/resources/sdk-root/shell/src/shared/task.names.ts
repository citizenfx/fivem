export const projectLoadingTaskName = 'project:loading';
export const projectCreatingTaskName = 'project:creating';

export const projectBuildingTaskName = 'project:building';
export enum ProjectBuildTaskStage {
  VerifyingBuildSite,
  RunningBuildCommands,
  PreparingBuildSite,
  DeployingToBuildSite,

  Done,
}
