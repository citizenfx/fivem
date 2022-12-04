import React from 'react';
import { clsx } from 'cfx/utils/clsx';
import s from './Center.module.scss';

export interface CenterProps {
  vertical?: boolean,
  horizontal?: boolean,

  className?: string,
  children?: React.ReactNode,
}

export const Center = React.forwardRef(function Center(props: CenterProps, ref: React.Ref<HTMLDivElement>) {
  const {
    vertical = false,
    horizontal = false,

    className,
    children,
  } = props;

  const rootClassName = clsx(s.root, className, {
    [s.vertical]: vertical,
    [s.horizontal]: horizontal,
  });

  return (
    <div ref={ref} className={rootClassName}>
      {children}
    </div>
  );
});
