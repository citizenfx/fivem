import React from 'react';

import { useContextualStyle } from 'cfx/ui/Style/Style';
import { ui } from 'cfx/ui/ui';
import { clsx } from 'cfx/utils/clsx';

import s from './Box.module.scss';

function quantOrRaw(value: number | string): string {
  if (typeof value === 'string') {
    return value;
  }

  return ui.q(value);
}

export interface BoxProps {
  noOverflow?: boolean;
  noShrink?: boolean;
  grow?: boolean;

  /**
   * Number values is treated like quant multiplier
   * String value used as is
   */
  width?: number | string;

  /**
   * Number values is treated like quant multiplier
   * String value used as is
   */
  height?: number | string;

  children?: React.ReactNode;

  className?: string;
  style?: React.CSSProperties;
}

export const Box = React.forwardRef(function Box(props: BoxProps, ref: React.Ref<HTMLDivElement>) {
  const {
    noOverflow = false,
    noShrink = false,
    grow = undefined,
    width,
    height,
    children,

    className,
  } = props;

  const rootClassName = clsx(s.root, className, {
    [s['no-overflow']]: noOverflow,
    [s['no-shrink']]: noShrink,
    [s['no-grow']]: grow === false,
    [s.grow]: grow === true,
  });

  const style: React.CSSProperties = {
    ...useContextualStyle(),
    ...(props.style || {}),
  };

  if (width || height) {
    if (width) {
      style.width = quantOrRaw(width);
    }

    if (height) {
      style.height = quantOrRaw(height);
    }
  }

  if (!style.width && grow === true) {
    style.width = '1px';
  }

  return (
    <div ref={ref} className={rootClassName} style={style}>
      {children}
    </div>
  );
});
