import { IDisposableObject } from './disposable';

export const queueMicrotask: (fn: () => any) => void = (globalThis || window).queueMicrotask
  || ((fn) => Promise.resolve().then(fn));

export function timeout(time: number): Promise<void> {
  return new Promise((resolve) => {
    setTimeout(resolve, time);
  });
}

export function idleCallback(timeoutArg?: number): Promise<void> {
  return new Promise((resolve) => {
    requestIdleCallback(resolve as any, { timeout: timeoutArg });
  });
}

export function animationFrame(): Promise<void> {
  return new Promise((resolve) => {
    requestAnimationFrame(resolve as any);
  });
}

export function resolveOrTimeout<T>(time: number, timeoutError: string, promise: Promise<T>): Promise<T> {
  return Promise.race([
    timeout(time).then(() => {
      throw new Error(timeoutError);
    }),
    promise,
  ]);
}

export class Deferred<T = undefined> {
  resolve: {
    (): void;
    (v: T): void;
  };

  reject: (err?: any) => void;

  promise: Promise<T>;

  constructor() {
    this.promise = new Promise<T>((resolve, reject) => {
      this.resolve = resolve as any;
      this.reject = reject;
    });
  }
}

/**
 * Like throttle, but also debounce but also smarter!
 *
 * Probably only useful for something using stateless async things, like async input validation
 *
 * Will only invoke callback with a result of the latest effective `runner` invokation
 *
 * `delay` adds, well, delay between `runner` invokations,
 * but only the latest effective invokation result will be passed to callback
 */
export class OnlyLatest<TWorkArgs extends any[], TWorkResult> implements IDisposableObject {
  private runningIndex = 0;

  private args: TWorkArgs | null = null;

  private get nextRunningIndex(): number {
    return ++this.runningIndex;
  }

  private disposed = false;

  private delayTimeout: ReturnType<typeof setTimeout> | null = null;

  public readonly run: (...args: TWorkArgs) => void;

  constructor(
    private runner: (...args: TWorkArgs) => Promise<TWorkResult>,
    private callback: (result: TWorkResult) => void,
    private delay?: number,
  ) {
    if (this.delay) {
      this.run = this.delayedRunner;
    } else {
      this.run = this.normalRunner;
    }
  }

  public dispose() {
    this.disposed = true;

    if (this.delayTimeout !== null) {
      clearTimeout(this.delayTimeout);
    }
  }

  private delayedRunPending = false;

  private readonly delayedRunner = (...args: TWorkArgs) => {
    this.args = args;

    if (!this.delayedRunPending) {
      this.delayedRunPending = true;

      this.delayTimeout = setTimeout(() => {
        if (this.disposed) {
          return;
        }

        this.delayedRunPending = false;
        this.delayTimeout = null;

        this.doRun();
      }, this.delay);
    }
  };

  private readonly normalRunner = (...args: TWorkArgs) => {
    this.args = args;

    this.doRun();
  };

  private readonly doRun = async () => {
    const currentIndex = this.nextRunningIndex;

    const args = this.args!;
    this.args = null;

    const result = await this.runner(...args);

    if (this.disposed) {
      return;
    }

    if (currentIndex === this.runningIndex) {
      this.callback(result);
    }
  };
}

// @ts-expect-error TS fails to understand that this code will either return or throw an error
// eslint-disable-next-line consistent-return
export async function retry<RetType>(attempts: number, fn: () => Promise<RetType>): Promise<RetType> {
  let attempt = 0;

  while (attempt++ <= attempts) {
    const lastAttempt = attempt === attempts;

    try {
      // eslint-disable-next-line no-await-in-loop
      return await fn();
    } catch (e) {
      if (lastAttempt) {
        throw e;
      }
    }
  }
}
