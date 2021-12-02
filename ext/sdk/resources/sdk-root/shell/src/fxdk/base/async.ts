import { IDisposable } from "./disposable";

export function disposableTimeout(fn: Parameters<typeof setTimeout>['0'], timeout: number): IDisposable {
  let disposed = false;

  const timer = setTimeout(() => {
    if (disposed) {
      return;
    }

    disposed = true;

    fn();
  }, timeout);

  return () => {
    if (disposed) {
      return;
    }

    disposed = true;

    clearTimeout(timer);
  };
}

export function disposableAnimationFrame(fn: FrameRequestCallback): IDisposable {
  let disposed = false;

  const rAF = requestAnimationFrame((dt) => {
    if (disposed) {
      return;
    }

    disposed = true;

    fn(dt);
  });

  return () => {
    if (disposed) {
      return;
    }

    disposed = true;

    cancelAnimationFrame(rAF);
  };
}

export async function awaitableTimeout(timeout: number): Promise<void> {
  return new Promise((resolve) => {
    setTimeout(resolve, timeout);
  });
}

export function disposableIdleCallback(fn: IdleRequestCallback): IDisposable {
  let disposed = false;

  const rIC = requestIdleCallback((deadline) => {
    if (disposed) {
      return;
    }

    disposed = true;

    fn(deadline);
  });

  return () => {
    if (disposed) {
      return;
    }

    disposed = true;

    cancelIdleCallback(rIC);
  };
}
