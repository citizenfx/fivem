import { IDisposableObject } from "fxdk/base/disposable";
import { Task } from "backend/task/task-reporter-service";
import { ServerStartRequest } from "./game-server-runtime";

export interface GameServerMode extends IDisposableObject {
  start(request: ServerStartRequest, task: Task): Promise<void>;
  stop(task: Task): Promise<void>;

  onStop(cb: (error?: Error) => void): IDisposableObject;
}
