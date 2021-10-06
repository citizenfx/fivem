import { Api } from "fxdk/browser/Api";
import { makeAutoObservable } from "mobx";
import { taskReporterApi } from "shared/api.events";
import { clamp01 } from "shared/math";
import { TaskData, TaskId } from "shared/task.types";
import { logger } from "utils/logger";
import { fastRandomId } from "utils/random";

const log = logger('TaskState');

class ClientTask<StageType = number> implements TaskData<StageType>{
  constructor(
    public readonly id: string,
    public readonly title: string,
    public text: string,
    public stage: StageType,
    public progress: number,
  ) {
    makeAutoObservable(this);
  }

  setText(text: string) {
    this.text = text;
  }

  setStage(stage: StageType) {
    this.stage = stage;
  }

  setProgress(progress: number) {
    this.progress = clamp01(progress);
  }
}

export const TaskState = new class TaskState {
  private tasks: Record<TaskId, TaskData> = {};

  constructor() {
    makeAutoObservable(this);

    Api.on(taskReporterApi.tasks, this.setTasks);
    Api.on(taskReporterApi.taskAdded, this.addTask);
    Api.on(taskReporterApi.taskChanged, this.updateTask);
    Api.on(taskReporterApi.taskDeleted, this.deleteTask);
  }

  public get values(): TaskData[] {
    return Object.values(this.tasks);
  }

  get(taskId: TaskId): TaskData | null {
    return this.tasks[taskId] || null;
  }

  getAll(): TaskData[] {
    return this.values;
  }

  async wrap(title: string, fn: (task: ClientTask<number>) => Promise<void> | void): Promise<void> {
    const id = `client-${fastRandomId()}`;

    const task = new ClientTask(id, title, '', 0, 0);

    this.addTask(task);

    try {
      await fn(task);
    } finally {
      this.deleteTask(id);
    }
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
      return;
    }

    delete this.tasks[taskId];
  };
}();
