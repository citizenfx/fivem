/**
 * !!!!
 * Defined during compilation
 * !!!!
 */
declare var __CFXUI_DEV__: boolean;
declare var __CFXUI_USE_SOUNDS__: boolean;

declare type SetTimeoutReturn = ReturnType<typeof setTimeout>;
declare type SetIntervalReturn = ReturnType<typeof setInterval>;
declare type SetImmediateReturn = ReturnType<typeof setImmediate>;
declare type RequestIdleCallbackReturn = ReturnType<typeof requestIdleCallback>;
declare type RequestAnimationFrameReturn = ReturnType<typeof requestAnimationFrame>;

declare type ChildrenProps = { children?: React.ReactNode };

declare type ChildrenfulReactFC = React.FC<ChildrenProps>;

declare namespace Intl {
  function getCanonicalLocales(locales: string | string[]): string[];
}

declare module '*.module.scss' {
  const classes: { readonly [key: string]: string };
  export default classes;
}
declare module '*.mp3' {
  const classes: string;
  export default classes;
}
declare module '*.ogg' {
  const classes: string;
  export default classes;
}
declare module '*.wav' {
  const classes: string;
  export default classes;
}
