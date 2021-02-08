export type TaskId = string;

export interface TaskData<StageType = number> {
  id: TaskId,
  text: string,
  title: string,
  stage: StageType,
  progress: number,
}
