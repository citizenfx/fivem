/// <reference types="react-scripts" />

declare var resizeGame: (width: number, height: number) => void;
declare var initRGDInput: () => boolean;
declare var setMouseButtonState: (button: number, state: boolean) => void;
declare var sendMouseWheel: (deltaY: number) => void;
declare var sendMousePos: (mx: number, my: number) => void;
declare var setKeyState: (vk: number, state: boolean) => void;
declare var setInputChar: (char: string) => void;

declare var sendGameClientEvent: (eventName: string, payload: string) => void | boolean;

declare var setWorldEditorMouse: (x: number, y: number) => void | boolean;
declare var setWorldEditorControls: (select: boolean, mode: number, local: boolean) => void | boolean;

declare var openDevTools: () => void;
declare var invokeNative: (native: string, ...arg: string[]) => void;
declare var renderGame: () => void;
declare var setFPSLimit: (limit: number) => void;
declare var fxdkSendApiMessage: (msg: string) => boolean;
declare var fxdkOpenSelectFolderDialog: (startPath: string, dialogTitle: string, callback: (selectedFolder: string | null) => void) => boolean;
declare var fxdkOpenSelectFileDialog: (startPath: string, dialogTitle: string, callback: (selectedFolder: string | null) => void) => boolean;

declare var dev: any;

declare var __ARCHETYPES_INDEX: any;

// Thanks TS...
declare var ResizeObserver: any;

declare namespace JSX {
  interface IntrinsicElements {
    'game-view': any,
  }
}

declare module '*.module.scss' {
  const classes: { [key: string]: string };
  export default classes;
}
declare module '*.raw.js' {
  const classes: string;
  export default classes;
}
