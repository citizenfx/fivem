import React from "react";
import { clsx } from "cfx/utils/clsx";
import s from './Dot.module.scss';

export interface DotProps {
  color?: 'primary',
}

export const Dot = React.forwardRef((props: DotProps, ref: React.Ref<HTMLDivElement>) => {
  const {
    color = 'primary',
  } = props;

  const rootClassName = clsx(s.root, s[`color-${color}`]);

  return (
    <div ref={ref} className={rootClassName} />
  );
});
