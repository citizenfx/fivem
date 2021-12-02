import * as React from 'react';
import classnames from 'classnames';
import { ToolbarState } from 'store/ToolbarState';
import { observer } from 'mobx-react-lite';
import { Resizer } from 'fxdk/ui/controls/Resizer/Resizer';
import s from './Toolbar.module.scss';

export const ToolbarResizer = observer(function ToolbarResizer() {
  const [toolbarResizing, setToolbarResizing] = React.useState(false);

  const handleResize = React.useCallback((deltaWidth: number) => {
    ToolbarState.setWidth(ToolbarState.width + deltaWidth);
  }, []);

  const handleResizeStart = React.useCallback(() => setToolbarResizing(true), []);
  const handleResizeStop = React.useCallback(() => setToolbarResizing(false), []);

  const triggerClassName = classnames(s.resizer, {
    [s.resizing]: toolbarResizing,
  });

  return (
    <Resizer
      onResize={handleResize}
      onResizingStart={handleResizeStart}
      onResizingStop={handleResizeStop}
    >
      {(handleMouseDown) => (
        <div
          className={triggerClassName}
          onMouseDown={handleMouseDown}
        />
      )}
    </Resizer>
  );
});
