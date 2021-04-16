import React from 'react';
import { ANY_MESSAGE, ApiMessageCallback, ApiMessageListener, onApiMessage, sendApiMessageCallback } from './api';
import { fastRandomId } from './random';
import { onWindowEvent, WindowEventListener } from './windowMessages';

export const useSid = (watchers: React.DependencyList = []) => {
  const initialSid = React.useMemo(fastRandomId, []);
  const sidRef = React.useRef(initialSid);

  React.useEffect(() => {
    sidRef.current = fastRandomId();
  }, watchers); // eslint-disable-line react-hooks/exhaustive-deps

  return sidRef.current;
};

export const useApiMessage = (type: string | typeof ANY_MESSAGE, cb: ApiMessageListener, deps: React.DependencyList = []) => {
  React.useEffect(() => onApiMessage(type, cb), deps); // eslint-disable-line react-hooks/exhaustive-deps
};

export const useWindowEvent = <T>(type: string, cb: WindowEventListener<T>, deps: React.DependencyList = []) => {
  React.useEffect(() => onWindowEvent(type, cb), [type, cb, ...deps]);
};

export const useCounter = (initial: number = 0) => {
  const [counter, setCounter] = React.useState<number>(initial);
  const counterRef = React.useRef(counter);
  counterRef.current = counter;

  const update = React.useCallback((diff: number) => {
    setCounter(counterRef.current + diff);
  }, []);

  const increment = React.useCallback(() => {
    setCounter(counterRef.current + 1);
  }, []);

  const decrement = React.useCallback(() => {
    setCounter(counterRef.current - 1);
  }, []);

  return [counter, increment, decrement, update];
};


// God damn you create-react-app
// https://github.com/facebook/create-react-app/issues/9515
// export type UseOpenFlagHook = [boolean, () => void, () => void, () => void];
export type UseOpenFlagHook = any;


export const useOpenFlag = (defaultValue: boolean = false): UseOpenFlagHook => {
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


export const useStore = <T>(defaultValue: Record<string, T>) => {
  const [sentinel, setSentinel] = React.useState({});

  const storeRef = React.useRef(defaultValue);

  const set = React.useCallback((id: string, item: T) => {
    storeRef.current[id] = item;

    setSentinel({});
  }, [setSentinel]);

  const get = React.useCallback((id: string) => {
    return storeRef.current[id];
  }, []);

  const remove = React.useCallback((id: string) => {
    delete storeRef.current[id];

    setSentinel({});
  }, [setSentinel]);

  return {
    store: storeRef.current,
    set,
    get,
    remove,
    ___sentinel: sentinel,
  };
};

export const useDebouncedCallback = <T extends any[], U extends any, R = (...args: T) => any>(
  cb: (...args: T) => U,
  timeout: number,
  watchers: React.DependencyList = [],
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

  return React.useCallback<any>(realCb, [...watchers]);
};

export interface UseOpenFolderSelectDialogOptions {
  startPath: string,
  dialogTitle: string,
  notOnlyFolders?: boolean,
}

export const useOpenFolderSelectDialog = (options: UseOpenFolderSelectDialogOptions, onSelected: (folderPath: string | null) => void) => {
  const { startPath, dialogTitle, notOnlyFolders = false } = options;

  const callbackRef = React.useRef(onSelected);
  callbackRef.current = onSelected;

  React.useEffect(() => () => callbackRef.current = null, []);

  return React.useCallback(() => {
    if (notOnlyFolders) {
      fxdkOpenSelectFileDialog(startPath, dialogTitle, (folderPath) => callbackRef.current?.(folderPath));
    } else {
      fxdkOpenSelectFolderDialog(startPath, dialogTitle, (folderPath) => callbackRef.current?.(folderPath));
    }
  }, [startPath, dialogTitle, notOnlyFolders]);
};

export const useSendApiMessageCallback = <Data, ResponseData>(type: string, callback: ApiMessageCallback<ResponseData>) => {
  const disposerRef = React.useRef<Function | null>(null);
  const callbackRef = React.useRef<ApiMessageCallback<ResponseData>>(callback);
  callbackRef.current = callback;

  React.useEffect(() => () => {
    disposerRef.current?.();

    callbackRef.current = null;
  }, []);

  return React.useCallback(async (data: Data) => {
    if (disposerRef.current) {
      return;
    }

    disposerRef.current = sendApiMessageCallback(type, data, (error, response) => {
      disposerRef.current = null;

      callbackRef.current?.(error, response as any);
    });
  }, [type]);
};
