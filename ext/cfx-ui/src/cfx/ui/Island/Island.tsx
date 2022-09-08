import { clsx } from 'cfx/utils/clsx';
import React from 'react';
import { ui } from '../ui';
import s from './Island.module.scss';

export enum IslandCorner {
  TopLeft = 1 << 0,
  TopRight = 1 << 1,
  BottomLeft = 1 << 2,
  BottomRight = 1 << 3,

  Top = IslandCorner.TopLeft | IslandCorner.TopRight,
  Left = IslandCorner.TopLeft | IslandCorner.BottomLeft,
  Right = IslandCorner.TopRight | IslandCorner.BottomRight,
  Bottom = IslandCorner.BottomLeft | IslandCorner.BottomRight,

  All = IslandCorner.Top | IslandCorner.Bottom,
}

export interface IslandProps {
  grow?: boolean,

  widthQ?: number,

  /**
   * Use IslandCorner enum to compose corner bits
   *
   * For example: two top corners would be `IslandCorner.TopLeft | IslandCorner.TopRight` or just `IslandCorner.Top`
   */
  straightCorner?: number,

  children?: React.ReactNode,
  className?: string,
}

export const Island = React.forwardRef(function Island(props: IslandProps, ref: React.Ref<HTMLDivElement>) {
  const {
    grow = false,
    widthQ = 0,
    straightCorner = 0,
    children,
    className,
  } = props;

  const rootClassName = clsx(s.root, className, {
    [s.grow]: grow,
    [s['no-round-border-top-left']]: straightCorner & IslandCorner.TopLeft,
    [s['no-round-border-top-right']]: straightCorner & IslandCorner.TopRight,
    [s['no-round-border-bottom-left']]: straightCorner & IslandCorner.BottomLeft,
    [s['no-round-border-bottom-right']]: straightCorner & IslandCorner.BottomRight,
  });

  const style: React.CSSProperties = {};

  if (widthQ > 0) {
    style.width = ui.q(widthQ);
  }

  return (
    <div className={rootClassName} style={style} ref={ref}>
      {children}
    </div>
  );
});
