import React from 'react';
import { useNavigationType } from 'react-router-dom';
import { dispose, IDisposableObject } from './disposable';

const Uninitialized = Symbol('Uninitialized');

export function useInstance<T, Args extends any[]>(init: (...args: Args) => T, ...args: Args): T {
  const ref = React.useRef<typeof Uninitialized | T>(Uninitialized);

  if (ref.current === Uninitialized) {
    ref.current = init(...args);
  }

  return ref.current;
}

export function useDisposableInstance<T extends IDisposableObject, Args extends any[]>(init: (...args: Args) => T, ...args: Args): T {
  const ref = React.useRef<typeof Uninitialized | T>(Uninitialized);

  if (ref.current === Uninitialized) {
    ref.current = init(...args);
  }

  React.useEffect(() => {
    if (ref.current === Uninitialized) {
      ref.current = init(...args);
    }

    return () => {
      if (ref.current !== Uninitialized) {
        dispose(ref.current);
        ref.current = Uninitialized;
      }
    };
  }, [])

  return ref.current;
}

export function useDynamicRef<T>(value: T): React.MutableRefObject<T> {
  const ref = React.useRef(value);
  ref.current = value;

  return ref;
}

export function useAnimationFrameFired(): boolean {
  const [fired, setFired] = React.useState(false);

  React.useEffect(() => {
    let rAF: null | number = requestAnimationFrame(() => {
      rAF = null;

      setFired(true);
    });

    return () => {
      if (rAF !== null) {
        cancelAnimationFrame(rAF);
      }
    };
  }, []);

  return fired;
}

export function useTimeoutFlag(time: number): boolean {
  const [timedout, setTimedout] = React.useState(false);

  const startedAtRef = React.useRef(0);

  React.useEffect(() => {
    // Intentionally out of deps
    if (timedout) {
      return;
    }

    let timer: SetTimeoutReturn | undefined;

    if (startedAtRef.current === 0) {
      startedAtRef.current = Date.now();
      timer = setTimeout(() => {
        timer = undefined;
        setTimedout(true);
      }, time);
    } else {
      const timeSpent = Date.now() - startedAtRef.current;

      if (timeSpent >= time) {
        setTimedout(true);
      } else {
        timer = setTimeout(() => {
          timer = undefined;
          setTimedout(true);
        }, time - timeSpent);
      }
    }

    return () => {
      if (timer) {
        clearTimeout(timer);
      }
    };
  }, [time]);

  return timedout;
}

export const useOpenFlag = (defaultValue: boolean = false): [boolean, () => void, () => void, () => void] => {
  const [isOpen, setIsOpen] = React.useState(defaultValue);

  const open = React.useCallback(() => {
    setIsOpen(true);
  }, []);
  const close = React.useCallback(() => {
    setIsOpen(false);
  }, []);

  const toggle = React.useCallback(() => {
    setIsOpen(!isOpen);
  }, [isOpen]);

  return [isOpen, open, close, toggle];
};

export function useWindowResize<T extends Function>(callback: T) {
  const callbackRef = useDynamicRef(callback);

  React.useEffect(() => {
    const handler = () => callbackRef.current();

    window.addEventListener('resize', handler);

    return () => window.removeEventListener('resize', handler);
  }, []);
}

export const useDebouncedCallback = <T extends any[], U extends any, R = (...args: T) => any>(
  cb: (...args: T) => U,
  timeout: number,
): R => {
  const cbRef = React.useRef(cb);
  const timerRef = React.useRef<any>();

  cbRef.current = cb;

  const realCb = (...args: T): any => {
    if (timerRef.current) {
      clearTimeout(timerRef.current);
    }

    timerRef.current = setTimeout(() => {
      timerRef.current = undefined;

      cbRef.current(...args);
    }, timeout);
  };

  React.useEffect(() => () => {
    if (timerRef.current) {
      clearTimeout(timerRef.current);
    }
  }, []);

  return React.useCallback<any>(realCb, []);
};

export function useKeyboardClose<T extends () => void>(callback: T) {
  const callbackRef = useDynamicRef((event: KeyboardEvent) => {
    if (!useKeyboardClose.isCloseEvent(event)) {
      return;
    }

    callback();
  });

  useGlobalKeyboardEvent(callbackRef);
}
useKeyboardClose.isCloseEvent = (event: KeyboardEvent) => {
  return event.key === 'Escape';
};

export function useGlobalKeyboardEvent<T extends (event: KeyboardEvent) => void>(
  callbackRef: React.MutableRefObject<T>,
  eventName: 'keydown' | 'keyup' | 'keypress' = 'keydown',
  capturing?: boolean
) {
  React.useEffect(() => {
    const handler = (event: KeyboardEvent) => {
      if (!useGlobalKeyboardEvent.shouldProcessEvent(event)) {
        return;
      }

      callbackRef.current(event);
    };

    window.addEventListener(eventName, handler, capturing);

    return () => {
      window.removeEventListener(eventName, handler, capturing);
    };
  }, [eventName, capturing]);
}
useGlobalKeyboardEvent.shouldProcessEvent = (event: KeyboardEvent) => {
  if (event.target instanceof Element) {
    if (event.target.hasAttribute('contenteditable')) {
      return false;
    }

    switch (event.target.tagName) {
      case 'INPUT':
      case 'SELECT':
      case 'TEXTAREA': {
        return false;
      }
    }
  }

  return true;
};


const scrollPositionMemo = new Map<any, number>();
export function useSavedScrollPositionForBackNav<T>(id: T): [number, (offset: number) => void] {
  const setScrollOffset = React.useCallback((scrollOffset: number) => {
    scrollPositionMemo.set(id, scrollOffset);
  }, []);

  const scrollOffset = useNavigationType() === 'POP'
    ? (scrollPositionMemo.get(id) || 0)
    : 0;

  return [scrollOffset, setScrollOffset];
}
