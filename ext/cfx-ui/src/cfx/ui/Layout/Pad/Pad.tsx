import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import s from './Pad.module.scss';

export interface PadProps {
  children?: React.ReactNode;
  className?: string;

  /**
   * 'normal' by default
   */
  size?: 'none' | 'small' | 'normal' | 'large' | 'xlarge';

  top?: boolean;
  left?: boolean;
  right?: boolean;
  bottom?: boolean;
}

export function Pad(props: PadProps) {
  const {
    top = false,
    left = false,
    right = false,
    bottom = false,

    size: offset = 'normal',

    children,
    className,
  } = props;

  const all = !(top || left || right || bottom);

  const rootClassName = clsx(s.root, className, s[`offset-${offset}`], {
    [s['only-top']]: all || top,
    [s['only-left']]: all || left,
    [s['only-right']]: all || right,
    [s['only-bottom']]: all || bottom,
  });

  return (
    <div className={rootClassName}>{children}</div>
  );
}
