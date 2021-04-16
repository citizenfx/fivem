import * as React from 'react';
import classnames from 'classnames';
import { BsArrowBarLeft, BsArrowBarRight } from 'react-icons/bs';
import { ToolbarState } from 'store/ToolbarState';
import { observer } from 'mobx-react-lite';
import s from './Toolbar.module.scss';

export const ToolbarTrigger = observer(function ToolbarTrigger() {
  const [toolbarResizing, setToolbarResizing] = React.useState(false);

  const triggerTitle = ToolbarState.isOpen
    ? 'Collapse FxDK toolbar'
    : 'Expand FxDK toolbar';
  const triggerIcon = ToolbarState.isOpen
    ? <BsArrowBarLeft />
    : <BsArrowBarRight />;

  const setToolbarResizingRef = React.useRef(setToolbarResizing);
  setToolbarResizingRef.current = setToolbarResizing;

  const mouseStateRef = React.useRef({
    x: 0,
    down: false,
    moved: false,
  });

  React.useEffect(() => {
    const handleMouseUp = () => {
      if (mouseStateRef.current.down) {
        window.document.body.classList.remove('reszie-sentinel-active');
        setToolbarResizingRef.current(false);

        mouseStateRef.current.down = false;
        mouseStateRef.current.moved = false;
      }
    };

    const handleMouseMove = (e) => {
      if (!mouseStateRef.current.down) {
        return;
      }

      if (mouseStateRef.current.x === e.screenX) {
        return;
      }

      mouseStateRef.current.moved = true;

      ToolbarState.setWidth(ToolbarState.width + (e.screenX - mouseStateRef.current.x));
      mouseStateRef.current.x = e.screenX;
    };

    window.addEventListener('mouseup', handleMouseUp);
    window.addEventListener('mousemove', handleMouseMove);

    return () => {
      window.document.body.classList.remove('reszie-sentinel-active');
      window.removeEventListener('mouseup', handleMouseUp);
      window.removeEventListener('mousemove', handleMouseMove);
    };
  }, []);

  const handleMouseDown = React.useCallback((e: React.MouseEvent) => {
    window.document.body.classList.add('reszie-sentinel-active');

    setToolbarResizing(true);

    mouseStateRef.current.down = true;
    mouseStateRef.current.x = e.screenX;
  }, [setToolbarResizing]);

  const handleMouseUp = React.useCallback(() => {
    if (mouseStateRef.current.down && !mouseStateRef.current.moved) {
      ToolbarState.toggle();
    }
  }, []);

  const triggerClassName = classnames(s.trigger, {
    [s.resizing]: toolbarResizing,
  });

  return (
    <button
      className={triggerClassName}
      title={triggerTitle}

      onMouseDown={handleMouseDown}
      onMouseUp={handleMouseUp}
    >
      {triggerIcon}
    </button>
  );
});
