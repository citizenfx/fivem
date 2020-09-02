import React from 'react';

export enum GameViewMode {
  observing = 0,
  controling = 1,
};

export interface GameViewProps {
  className?: string,
  mode?: GameViewMode,
};

export const GameView = React.memo(({ className, mode = GameViewMode.observing }: GameViewProps) => {
  const ref = React.useRef<any>(null);

  React.useEffect(() => {
    if (ref.current) {
      ref.current.mode = mode;
    }
  }, [mode]);

  return (
    <game-view ref={ref} className={className}></game-view>
  );
});
