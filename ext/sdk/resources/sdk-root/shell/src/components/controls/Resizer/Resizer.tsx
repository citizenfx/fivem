import React from 'react';

export interface ResizerProps {
  onResize(deltaWidth: number): void,
  resizeEffect?(): () => void,

  children(handleMouseDown: any): React.ReactElement,

  onResizingStart?(): void,
  onResizingStop?(): void,
}

const defaultState = {
  x: 0,
  down: false,
  moved: false,

  changeCb(d: number) {},
  startCb() {},
  stopCb() {},
};

const noop = () => {};

export function Resizer(props: ResizerProps) {
  const {
    children,
    onResize,
    onResizingStart = noop,
    onResizingStop = noop,
  } = props;

  const stateRef = React.useRef(defaultState);

  stateRef.current.changeCb = onResize;
  stateRef.current.startCb = onResizingStart;
  stateRef.current.stopCb = onResizingStop;

  React.useEffect(() => {
    const handleMouseUp = () => {
      if (stateRef.current.down) {
        window.document.body.classList.remove('resize-sentinel-active');
        stateRef.current.stopCb();

        stateRef.current.down = false;
        stateRef.current.moved = false;
      }
    };

    const handleMouseMove = (e) => {
      if (!stateRef.current.down) {
        return;
      }

      if (stateRef.current.x === e.screenX) {
        return;
      }

      stateRef.current.moved = true;

      stateRef.current.changeCb(e.screenX - stateRef.current.x);

      stateRef.current.x = e.screenX;
    };

    window.addEventListener('mouseup', handleMouseUp);
    window.addEventListener('mousemove', handleMouseMove);

    return () => {
      window.document.body.classList.remove('resize-sentinel-active');
      window.removeEventListener('mouseup', handleMouseUp);
      window.removeEventListener('mousemove', handleMouseMove);
    };
  }, []);

  const handleMouseDown = React.useCallback((e: React.MouseEvent) => {
    window.document.body.classList.add('resize-sentinel-active');

    stateRef.current.startCb();

    stateRef.current.down = true;
    stateRef.current.x = e.screenX;
  }, []);

  return children(handleMouseDown);
};
