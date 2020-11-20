import React from 'react';
import { ANY_MESSAGE, ApiMessageListener, onApiMessage } from './api';
import { fastRandomId } from './random';

export const useSid = (watchers: any[] = []) => {
  const initialSid = React.useMemo(fastRandomId, []);
  const sidRef = React.useRef(initialSid);

  React.useEffect(() => {
    sidRef.current = fastRandomId();
  }, watchers); // eslint-disable-line react-hooks/exhaustive-deps

  return sidRef.current;
};

export const useApiMessage = (type: string | typeof ANY_MESSAGE, cb: ApiMessageListener, watchers: any[] = []) => {
  React.useEffect(() => onApiMessage(type, cb), watchers); // eslint-disable-line react-hooks/exhaustive-deps
};

export const useSidApiMessage = (sid: string, type: string, cb: ApiMessageListener, watchers: any[] = []) => {
  useApiMessage(`${type}(${sid})`, cb, watchers); // eslint-disable-line react-hooks/exhaustive-deps
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
