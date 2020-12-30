type P<T> = Promise<T>;

export function concurrently<T1>(p1: P<T1>): P<[T1]>;
export function concurrently<T1, T2>(p1: P<T1>, p2: P<T2>): P<[T1, T2]>;
export function concurrently<T1, T2, T3>(p1: P<T1>, p2: P<T2>, p3: P<T3>): P<[T1, T2, T3]>;
export function concurrently<T1, T2, T3, T4>(p1: P<T1>, p2: P<T2>, p3: P<T3>, p4: P<T4>): P<[T1, T2, T3, T4]>;
export function concurrently<T1, T2, T3, T4, T5>(p1: P<T1>, p2: P<T2>, p3: P<T3>, p4: P<T4>, p5: P<T5>): P<[T1, T2, T3, T4, T5]>;
export function concurrently(...promises: P<any>[]): P<any[]> {
  return Promise.all(promises);
}
