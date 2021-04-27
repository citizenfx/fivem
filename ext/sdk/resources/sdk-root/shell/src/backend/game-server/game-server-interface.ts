import { DisposableObject } from "backend/disposable-container";
import { Task } from "backend/task/task-reporter-service";
import { ServerStartRequest } from "./game-server-runtime";

export interface GameServerMode extends DisposableObject {
  start(request: ServerStartRequest, task: Task): Promise<void>;
  stop(task: Task): Promise<void>;

  onStop(cb: (error?: Error) => void): DisposableObject;
}
