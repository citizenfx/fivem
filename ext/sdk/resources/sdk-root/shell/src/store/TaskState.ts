import { makeAutoObservable } from "mobx";
import { taskReporterApi } from "shared/api.events";
import { TaskData, TaskId } from "shared/task.types";
import { onApiMessage, sendApiMessage } from "utils/api";
import { logger } from "utils/logger";

const log = logger('TaskState');

export const TaskState = new class TaskState {
  private tasks: TaskData[] = [];
  private indices: Record<TaskId, number>;

  constructor() {
    makeAutoObservable(this);

    // So mobx don't even attempt to make this object observable
    this.indices = Object.create(null);

    onApiMessage(taskReporterApi.tasks, this.setTasks);
    onApiMessage(taskReporterApi.taskAdded, this.addTask);
    onApiMessage(taskReporterApi.taskChanged, this.updateTask);
    onApiMessage(taskReporterApi.taskDeleted, this.deleteTask);
  }

  public ack() {
    sendApiMessage(taskReporterApi.ackTasks);
  }

  get(taskId: TaskId): TaskData | null {
    return this.tasks[this.indices[taskId]] || null;
  }

  getAll(): TaskData[] {
    return this.tasks;
  }

  private setTasks = (tasks: TaskData[]) => {
    this.indices = tasks.reduce((acc, task, index) => {
      acc[task.id] = index;
      return acc;
    }, Object.create(null));

    this.tasks = tasks;
  };

  private addTask = (task: TaskData) => {
    if (this.indices[task.id] > -1) {
      return log('Attempt to add task that already exist', task, this.tasks);
    }

    this.indices[task.id] = this.tasks.length;
    this.tasks.push(task);
  };

  private updateTask = (task: TaskData) => {
    const index = this.indices[task.id];
    if (index > -1) {
      this.tasks[index] = task;
    } else {
      log('Can not update task as its index is unknown', task.id);
    }
  };

  private deleteTask = (taskId: TaskId) => {
    const index = this.indices[taskId];
    if (index > -1) {
      this.tasks.splice(index, 1);
      delete this.indices[taskId];
    } else {
      log('Can not delete task as its index is unknown', taskId);
    }
  };
};
