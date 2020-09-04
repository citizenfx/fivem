/// <reference types="react-scripts" />

declare var openDevTools: () => void;
declare var invokeNative: (native: string, ...arg: string[]) => void;

declare namespace JSX {
  interface IntrinsicElements {
    'game-view': any,
  }
}

declare module '*.module.scss' {
  const classes: { [key: string]: string };
  export default classes;
}
