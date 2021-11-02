import { makeAutoObservable } from "mobx";
import { Project } from "fxdk/project/browser/state/project";

export class PathState {
  get isExpanded(): boolean {
    const state = Project.pathsState[this.entryPath];
    if (state === undefined) {
      return this.defaultOpen;
    }

    return state;
  }

  constructor(private entryPath: string, private defaultOpen = false) {
    makeAutoObservable(this);
  }

  readonly expand = () => {
    Project.setPathState(this.entryPath, true);
  };

  readonly collapse = () => {
    Project.setPathState(this.entryPath, false);
  };

  readonly toggle = () => {
    Project.setPathState(this.entryPath, !this.isExpanded);
  };
}
