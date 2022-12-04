import React from 'react';
import { clsx } from 'cfx/utils/clsx';
import s from './Badge.module.scss';

export type BadeColor =
  | 'default'
  | 'primary'
  | 'success'
  | 'warning'
  | 'error';

export interface BadgeProps {
  color?: BadeColor,

  children?: React.ReactNode,
  className?: string,
}

export const Badge = React.forwardRef(function Badge(props: BadgeProps, ref: React.Ref<HTMLDivElement>) {
  const {
    color = 'default',

    children,
    className,
  } = props;

  const rootClassName = clsx(s.root, className, s[color]);

  return (
    <div ref={ref} className={rootClassName}>
      {children}
    </div>
  );
});
