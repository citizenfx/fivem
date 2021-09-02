import { makeAutoObservable } from "mobx";
import { taskReporterApi } from "shared/api.events";
import { TaskData, TaskId } from "shared/task.types";
import { onApiMessage, sendApiMessage } from "utils/api";
import { logger } from "utils/logger";

const log = logger('TaskState');

export const TaskState = new class TaskState {
  private tasks: Record<TaskId, TaskData> = {};

  constructor() {
    makeAutoObservable(this);

    onApiMessage(taskReporterApi.tasks, this.setTasks);
    onApiMessage(taskReporterApi.taskAdded, this.addTask);
    onApiMessage(taskReporterApi.taskChanged, this.updateTask);
    onApiMessage(taskReporterApi.taskDeleted, this.deleteTask);
  }

  public get values(): TaskData[] {
    return Object.values(this.tasks);
  }

  public ack() {
    sendApiMessage(taskReporterApi.ackTasks);
  }

  get(taskId: TaskId): TaskData | null {
    return this.tasks[taskId] || null;
  }

  getAll(): TaskData[] {
    return this.values;
  }

  private setTasks = (tasks: TaskData[]) => {
    this.tasks = tasks.reduce((acc, task) => {
      acc[task.id] = task;

      return acc;
    }, {});
  };

  private addTask = (task: TaskData) => {
    if (this.tasks[task.id]) {
      return log('Attempt to add task that already exist', task, this.tasks);
    }

    this.tasks[task.id] = task;
  };

  private updateTask = (task: TaskData) => {
    if (!this.tasks[task.id]) {
      log('Can not update task as its index is unknown', task.id);
      return;
    }

    this.tasks[task.id] = task;
  };

  private deleteTask = (taskId: TaskId) => {
    if (!this.tasks[taskId]) {
      log('Can not delete task as its index is unknown', taskId);
      return;
    }

    delete this.tasks[taskId];
  };
}();
