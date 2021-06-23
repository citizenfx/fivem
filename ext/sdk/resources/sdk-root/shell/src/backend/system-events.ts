export type SystemEventHandler<PayloadType extends any> = (payload?: PayloadType) => void;

export function onSystemEvent<PayloadType extends any>(eventName: string, handler: SystemEventHandler<PayloadType>): () => void {
  on(eventName, handler);

  return () => RemoveEventHandler(eventName, handler);
}

export function emitSystemEvent<PayloadType extends any>(eventName: string, payload?: PayloadType): void {
  emit(eventName, payload);
}
