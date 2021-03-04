export interface Executor {
  execute<T extends any[], U>(fn: (...args: T) => U): U,
}

export interface BoundExecutor<RetType> {
  execute(): RetType,
}
