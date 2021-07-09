import { makeAutoObservable } from "mobx";

export enum WETool {
  Patches,
  Additions,
  Properties,
  AddObject,
  StatusCenter,
}

type WEToolSide = 'left' | 'right';

const LEFT_SIDE_TOOL = 'left';
const RIGHT_SIDE_TOOL = 'right';

export const TOOL_SIDE: Record<WETool, WEToolSide> = {
  [WETool.Patches]: LEFT_SIDE_TOOL,
  [WETool.Additions]: LEFT_SIDE_TOOL,
  [WETool.Properties]: LEFT_SIDE_TOOL,
  [WETool.AddObject]: LEFT_SIDE_TOOL,
  [WETool.StatusCenter]: RIGHT_SIDE_TOOL,
};

export const WorldEditorToolbarState = new class WorldEditorToolbarState {
  private activeTool: Record<WEToolSide, WETool | null> = {
    [LEFT_SIDE_TOOL]: null,
    [RIGHT_SIDE_TOOL]: null,
  };

  constructor() {
    makeAutoObservable(this);
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
