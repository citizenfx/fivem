/* eslint-disable no-var */
/**
 * !!!!
 * Defined during compilation
 * !!!!
 */
declare var __CFXUI_DEV__: boolean;
declare var __CFXUI_USE_SOUNDS__: boolean;
declare var __CFXUI_CNL_ENDPOINT__: string;
declare var __CFXUI_SENTRY_DSN__: string;
declare var __CFXUI_SENTRY_RELEASE__: string;

declare type SetTimeoutReturn = ReturnType<typeof setTimeout>;
declare type SetIntervalReturn = ReturnType<typeof setInterval>;
declare type SetImmediateReturn = ReturnType<typeof setImmediate>;
declare type RequestIdleCallbackReturn = ReturnType<typeof requestIdleCallback>;
declare type RequestAnimationFrameReturn = ReturnType<typeof requestAnimationFrame>;

declare interface ChildrenProps {
  children?: React.ReactNode;
}

declare type ChildrenfulReactFC = React.FC<ChildrenProps>;

declare namespace Intl {
  function getCanonicalLocales(locales: string | string[]): string[];
}

declare module '*.module.scss' {
  const classes: Readonly<Record<string, string>>;
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
