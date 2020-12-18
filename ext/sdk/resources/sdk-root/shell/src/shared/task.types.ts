export type TaskId = string;

export interface TaskData {
  id: TaskId,
  text: string,
  title: string,
  progress: number,
}
