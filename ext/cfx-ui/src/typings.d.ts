/// <reference path="../node_modules/protobufjs/stub-long.d.ts" />
/// <reference path="../node_modules/protobufjs/stub-node.d.ts" />


/* SystemJS module definition */
declare var module: NodeModule;
interface NodeModule {
  id: string;
}

declare module '*.json' {
    const value: any;
    export default value;
}