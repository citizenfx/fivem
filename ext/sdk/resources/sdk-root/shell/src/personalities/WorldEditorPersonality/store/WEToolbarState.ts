import { makeAutoObservable } from "mobx";

export enum WETool {
  Patches,
  Additions,
  AddObject,
  StatusCenter,
  Environment,
  Properties,
  Settings,
}

export enum WEToolSide {
  LEFT_TOP = 'left_top',
  LEFT_BOTTOM = 'left_bottom',
  RIGHT = 'right',
}

export const TOOL_SIDE: Record<WETool, WEToolSide> = {
  [WETool.Patches]: WEToolSide.LEFT_TOP,
  [WETool.Additions]: WEToolSide.LEFT_TOP,
  [WETool.AddObject]: WEToolSide.LEFT_TOP,

  [WETool.Properties]: WEToolSide.LEFT_BOTTOM,

  [WETool.Settings]: WEToolSide.RIGHT,
  [WETool.Environment]: WEToolSide.RIGHT,
  [WETool.StatusCenter]: WEToolSide.RIGHT,
};

export const WEToolbarState = new class WEToolbarState {
  private activeTool: Record<WEToolSide, WETool | null> = {
    [WEToolSide.LEFT_TOP]: null,
    [WEToolSide.LEFT_BOTTOM]: null,
    [WEToolSide.RIGHT]: null,
  };

  constructor() {
    makeAutoObservable(this);
  }

  isSideActive(side: WEToolSide): boolean {
    return this.activeTool[side] !== null;
  }

  isToolOpen(tool: WETool): boolean {
    return this.activeTool[TOOL_SIDE[tool]] === tool;
  }

  openTool(tool: WETool) {
    this.activeTool[TOOL_SIDE[tool]] = tool;
  }

  closeTool(tool: WETool) {
    this.activeTool[TOOL_SIDE[tool]] = null;
  }

  toggleTool(tool: WETool) {
    if (this.isToolOpen(tool)) {
      this.closeTool(tool);
    } else {
      this.openTool(tool);
    }
  }
};
