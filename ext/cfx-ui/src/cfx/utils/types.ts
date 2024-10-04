export type NotFunction =
  | symbol
  | string
  | number
  | object
  | boolean
  | any[]
  | Map<any, any>
  | Set<any>
  | undefined
  | null
  | void;

export type Optional<T> = void | undefined | T;

export type InvokablesRecord = Record<string, (...args: any[]) => any>;

export namespace RichEvent {
  export type Descriptor<T, Payload> = T & {
    $__payload__: Payload;
  };

  export type Payload<D extends Descriptor<string, any>> = D['$__payload__'];

  export function define<P = object>(eventName: string): Descriptor<string, P> {
    return eventName as any;
  }
}
