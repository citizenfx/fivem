import { makeAutoObservable } from "mobx";

function clampToolbarWidth(width: number): number {
  return Math.max(250, Math.min(document.body.offsetWidth / 2, width));
};

export const ToolbarState = new class ToolbarState {
  public width = clampToolbarWidth(parseInt(localStorage.toolbarWidth, 0) || 430);

  public get cssVariables(): React.CSSProperties {
    return {
      '--toolbar-width': `${this.width}px`,
    } as any;
  }

  constructor() {
    makeAutoObservable(this);
  }

  setWidth(width: number) {
    this.width = clampToolbarWidth(width);

    localStorage.toobarWidth = this.width;
  }
}();
