import { AsyncSingleEventEmitter } from "utils/singleEventEmitter";
import { ProjectRuntime } from "./project-runtime/project-runtime";
import { ProjectStateRuntime } from "./project-runtime/project-state-runtime";

export namespace ProjectEvents {
  export const Created = new AsyncSingleEventEmitter<ProjectStateRuntime>();
  export const Opened = new AsyncSingleEventEmitter<ProjectStateRuntime>();

  /**
   * Instead of listening on both Created and Opened one can listen on this event
   */
  export const BeforeLoad = new AsyncSingleEventEmitter<ProjectStateRuntime>();
  export const Loaded = new AsyncSingleEventEmitter<ProjectRuntime>();

  export const BeforeUnload = new AsyncSingleEventEmitter<ProjectRuntime>();
}
