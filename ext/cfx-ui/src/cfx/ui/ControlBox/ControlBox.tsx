import React from "react";
import { clsx } from "cfx/utils/clsx";
import s from './ControlBox.module.scss';

export type ControlBoxSize =
  | 'small'
  | 'normal'
  | 'large';

export interface ControlBoxProps {
  size?: ControlBoxSize,
  className?: string,
  children: React.ReactNode,
}

export const ControlBox = React.forwardRef((props: ControlBoxProps, ref: React.Ref<HTMLDivElement>) => {
  const {
    size = 'normal',
    className,
    children,
  } = props;

  const rootClassName = clsx(s.root, className, s[size]);

  return (
    <div ref={ref} className={rootClassName}>
      {children}
    </div>
  );
});
