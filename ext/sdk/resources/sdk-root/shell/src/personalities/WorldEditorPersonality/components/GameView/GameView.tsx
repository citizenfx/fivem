import React from 'react';
import { observer } from 'mobx-react-lite';
import { GameViewRenderer } from './GameView.utils';
import s from './GameView.module.scss';

export const GameView = observer(function GameView() {
  const canvasRef = React.useRef();

  React.useEffect(() => {
    let width: number;
    let height: number;
    let debounce: any;

    const renderer = new GameViewRenderer(canvasRef.current);
    const resizeObserver = new ResizeObserver((entries) => {
      for (const entry of entries) {
        width = entry.contentRect.width;
        height = entry.contentRect.height;

        if (debounce) {
          clearTimeout(debounce);
        }

        debounce = setTimeout(() => {
          renderer.resize(width, height);
        }, 100);
      }
    });

    resizeObserver.observe(canvasRef.current);

    return () => {
      resizeObserver.disconnect();
      renderer.destroy();
    };
  }, []);

  return (
    <canvas className={s.root} ref={canvasRef} />
  );
});
