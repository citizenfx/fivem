export const debounce = <T extends Function>(fn: T, timeout: number): T & { cancel: () => void } => {
  // any as should be suitable both for node and browser
  let timer: any = null;

  const newFn = (...args) => {
    if (timer) {
      clearTimeout(timer);
    }

    timer = setTimeout(() => {
      timer = null;

      fn(...args);
    }, timeout);
  };

  newFn.cancel = () => clearTimeout(timer);

  return newFn as any;
};

// Throttle, but not really :p
// Will also fire after timeout
export const throttle = <T extends Function>(fn: T, timeout: number): T & { cancel: () => void } => {
  // any as should be suitable both for node and browser
  let timer: any = null;
  let canRun = true;

  const newFn = (...args) => {
    if (canRun) {
      fn(...args);
    }

    if (timer) {
      clearTimeout(timer);
    }

    canRun = false;
    timer = setTimeout(() => {
      timer = null;
      canRun = true;
      fn(...args);
    }, timeout);
  };

  newFn.cancel = () => clearTimeout(timer);

  return newFn as any;
};

export function notNull<T>(e: T | null): e is T {
  return !!e;
}
