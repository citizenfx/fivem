import { useDynamicRef } from '@cfx-dev/ui-components';
import React from 'react';
import { useNavigationType } from 'react-router-dom';

import { useIntlService } from 'cfx/common/services/intl/intl.service';

import { dispose, IDisposableObject } from './disposable';

const Uninitialized = Symbol('Uninitialized');

export function useDisposableInstance<T extends IDisposableObject, Args extends any[]>(
  init: (...args: Args) => T,
  ...args: Args
): T {
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
  }, []);

  return ref.current;
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

export function useTimeoutFlag(timeoutMS: number): boolean {
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
      }, timeoutMS);
    } else {
      const timeSpent = Date.now() - startedAtRef.current;

      if (timeSpent >= timeoutMS) {
        setTimedout(true);
      } else {
        timer = setTimeout(() => {
          timer = undefined;
          setTimedout(true);
        }, timeoutMS - timeSpent);
      }
    }

    return () => {
      if (timer) {
        clearTimeout(timer);
      }
    };
  }, [timeoutMS]);

  return timedout;
}

export const useOpenFlag = (defaultValue = false): [boolean, () => void, () => void, () => void] => {
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

export function useWindowResize<T extends () => void>(callback: T) {
  const callbackRef = useDynamicRef(callback);

  React.useEffect(() => {
    const handler = () => callbackRef.current();

    window.addEventListener('resize', handler);

    return () => window.removeEventListener('resize', handler);
  }, []);
}

export function useElementResize<T extends HTMLElement, C extends () => void>(ref: React.RefObject<T>, callback: C) {
  const callbackRef = useDynamicRef(callback);

  React.useEffect(() => {
    if (!ref.current) {
      return;
    }

    const observer = new ResizeObserver(() => callbackRef.current());

    observer.observe(ref.current);

    return () => {
      observer.disconnect();
    };
  }, [ref]);
}

export const useDebouncedCallback = <T extends any[], U, R = (...args: T) => any>(
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

  React.useEffect(
    () => () => {
      if (timerRef.current) {
        clearTimeout(timerRef.current);
      }
    },
    [],
  );

  return React.useCallback<any>(realCb, []);
};

const scrollPositionMemo = new Map<any, number>();
export function useSavedScrollPositionForBackNav<T>(id: T): [number, (offset: number) => void] {
  const setScrollOffset = React.useCallback((scrollOffset: number) => {
    scrollPositionMemo.set(id, scrollOffset);
  }, []);

  const scrollOffset = useNavigationType() === 'POP'
    ? scrollPositionMemo.get(id) || 0
    : 0;

  return [scrollOffset, setScrollOffset];
}

export function useBoundingClientRect<T extends HTMLElement>(ref: React.RefObject<T>): DOMRect | null {
  const [rect, setRect] = React.useState<DOMRect | null>(null);

  const recalculate = React.useCallback(() => {
    if (!ref.current) {
      setRect(null);
    } else {
      setRect(DOMRect.fromRect(ref.current.getBoundingClientRect()));
    }
  }, [ref]);

  useWindowResize(recalculate);
  useElementResize(ref, recalculate);

  React.useEffect(() => {
    recalculate();
  }, [recalculate]);

  return rect;
}

export function useServerCountryTitle(locale?: string, localeCountry?: string): string {
  const IntlService = useIntlService();

  const countryTitle
    = localeCountry === '001' || localeCountry === 'AQ' || localeCountry === 'aq'
      ? ''
      : IntlService.defaultDisplayNames.of((locale ?? localeCountry) || '');

  return countryTitle || '';
}
