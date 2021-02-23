/// <reference types="react-scripts" />

declare var openDevTools: () => void;
declare var invokeNative: (native: string, ...arg: string[]) => void;
declare var renderGame: () => void;
declare var setFPSLimit: (limit: number) => void;
declare var fxdkSendApiMessage: (msg: string) => boolean;
declare var fxdkOpenSelectFolderDialog: (startPath: string, dialogTitle: string, callback: (selectedFolder: string | null) => void) => boolean;
declare var fxdkOpenSelectFileDialog: (startPath: string, dialogTitle: string, callback: (selectedFolder: string | null) => void) => boolean;

declare var dev: any;

declare namespace JSX {
  interface IntrinsicElements {
    'game-view': any,
  }
}

declare module '*.module.scss' {
  const classes: { [key: string]: string };
  export default classes;
}
