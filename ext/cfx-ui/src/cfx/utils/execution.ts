export function debounce<A extends any[]>(fn: (...args: A) => void, time: number): (...args: A) => void {
  let timer: SetTimeoutReturn | null = null;

  return (...args) => {
    if (timer !== null) {
      clearTimeout(timer);
    }

    timer = setTimeout(() => {
      timer = null;
      fn(...args);
    }, time);
  };
}

export function throttle<A extends any[]>(fn: (...args: A) => void, time: number): (...args: A) => void {
  let timer: SetTimeoutReturn | null = null;

  return (...args) => {
    if (timer !== null) {
      return;
    }

    timer = setTimeout(() => {
      timer = null;
    }, time);
    fn(...args);
  };
}
