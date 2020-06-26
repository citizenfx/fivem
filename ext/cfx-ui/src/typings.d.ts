/* SystemJS module definition */
declare var module: NodeModule;
interface NodeModule {
  id: string;
}

declare module '*.json' {
    const value: any;
    export default value;
}

declare module 'webpack-extended-import-glob-loader!*' {
    const value: any;
    export default value;
}