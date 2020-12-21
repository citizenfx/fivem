import { ApiClient } from "backend/api/api-client";
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientEvent } from "backend/api/api-decorators";
import { inject, injectable } from "inversify";
import { taskReporterApi } from "shared/api.events";
import { TaskData, TaskId } from "shared/task.types";
import { throttle } from "shared/utils";
import { fastRandomId } from "utils/random";

@injectable()
export class TaskReporterService implements ApiContribution {
  getId() {
    return 'TaskReporterService';
  }

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  protected tasks: Record<TaskId, Task> = {};

  create(title: string): Task {
    return this.startWithId(fastRandomId(), title);
  }

  /**
   * Only one task with given name can exist simultaneously
   */
  createNamed(name: TaskId, title: string): Task {
    if (this.tasks[name]) {
      throw new Error(`Task with name ${name} already running`);
    }

    return this.startWithId(name, title);
  }

  async wrap<T = void>(title: string, fn: (task: Task) => Promise<T>): Promise<T> {
    const task = this.create(title);

    try {
      return await fn(task);
    } catch (e) {
      throw e;
    } finally {
      task.done();
    }
  }

  private startWithId(id: TaskId, title: string): Task {
    const task = new Task(
      this.apiClient,
      title,
      () => {
        delete this.tasks[id];
        this.apiClient.emit(taskReporterApi.taskDeleted, id);
      },
      id,
    );

    this.tasks[id] = task;

    this.apiClient.emit(taskReporterApi.taskAdded, task.toTaskData());

    return task;
  }

  @handlesClientEvent(taskReporterApi.ackTasks)
  ackTasks() {
    const jsonified = Object.values(this.tasks).map((task) => task.toTaskData());

    this.apiClient.emit(taskReporterApi.tasks, jsonified);
  }
}

export class Task {
  protected text: string = '';
  protected progress: string = '';

  protected isDone = false;

  constructor(
    protected apiClient: ApiClient,
    protected title: string,
    protected onDone: () => void,
    protected readonly id: TaskId,
  ) { }

  setText(text: string) {
    if (this.text !== text) {
      this.text = text;

      this.emitChange();
    }
  }

  /**
   * @param progress bounds are 0-1, other values will be clamped to that
   */
  setProgress(progress: number) {
    const progressString = Math.max(0, Math.min(1, progress)).toFixed(2);

    if (this.progress !== progressString) {
      this.progress = progressString;

      this.emitChange();
    }
  }

  done() {
    if (!this.isDone) {
      this.isDone = true;

      this.onDone();
    }
  }

  toTaskData(): TaskData {
    return {
      id: this.id,
      text: this.text,
      title: this.title,
      progress: Number(this.progress),
    };
  }

  private emitChange() {
    if (this.isDone) {
      return;
    }

    this.doEmitChanges();
  }

  private doEmitChanges = throttle(() => {
    this.apiClient.emit(taskReporterApi.taskChanged, this.toTaskData());
  }, 16);
}
