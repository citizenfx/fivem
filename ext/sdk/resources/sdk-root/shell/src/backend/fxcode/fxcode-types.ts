export interface IKeybinding {
  key: string,
  command: string,
}

export interface FXCodeImportConfig {
  extensions: Record<string, boolean>,
  settings: Record<string, unknown>,
  keybindings: IKeybinding[],
}
