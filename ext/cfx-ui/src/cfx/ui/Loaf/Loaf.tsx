import React from "react";
import { clsx } from "cfx/utils/clsx";
import s from './Loaf.module.scss';

export type LoafSize =
  | 'small'
  | 'normal'
  | 'large';

export type LoafColor =
  | 'normal'
  | 'gold'
  | 'error';

export interface LoafProps {
  size?: LoafSize,
  color?: LoafColor,
  bright?: boolean,
  className?: string,
  children: React.ReactNode,
}

export const Loaf = React.forwardRef(function Loaf(props: LoafProps, ref: React.Ref<HTMLDivElement>) {
  const {
    size = 'normal',
    color = 'normal',
    bright = false,
    children,
    className,
  } = props;

  const rootClassName = clsx(s.root, s[`size-${size}`], s[`color-${color}`], className, {
    [s.bright]: bright,
  });

  return (
    <div ref={ref} className={rootClassName}>
      {children}
    </div>
  );
});
