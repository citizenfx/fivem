import React from "react";
import { clsx } from 'cfx/utils/clsx';
import s from './Flex.module.scss';

export interface FlexProps {
  fullWidth?: boolean,
  fullHeight?: boolean,

  centered?: boolean | 'axis' | 'cross-axis',
  vertical?: boolean,
  repell?: boolean,
  stretch?: boolean,
  wrap?: boolean,
  spaceBetween?: boolean,
  alignToEnd?: boolean,

  gap?: 'none' | 'thin' | 'small' | 'normal' | 'large' | 'xlarge',

  children?: React.ReactNode,
  className?: string,
}

export const Flex = React.forwardRef(function Flex(props: FlexProps, ref: React.Ref<HTMLDivElement>) {
  const {
    fullWidth = false,
    fullHeight = false,
    vertical = false,
    centered = false,
    repell = false,
    stretch = false,
    wrap = false,
    alignToEnd = false,
    spaceBetween = false,
    gap = 'normal',
    children,
    className,
  } = props;

  const rootClassName = clsx(s.root, className, s[`gap-${gap}`], {
    [s['full-width']]: fullWidth,
    [s['full-height']]: fullHeight,
    [s.centered]: centered === true,
    [s['centered-axis']]: centered === 'axis',
    [s['centered-cross-axis']]: centered === 'cross-axis',
    [s.vertical]: vertical,
    [s.horizontal]: !vertical,
    [s.repell]: repell,
    [s.stretch]: stretch,
    [s.wrap]: wrap,
    [s['align-to-end']]: alignToEnd,
    [s['space-between']]: spaceBetween,
  });

  return (
    <div ref={ref} className={rootClassName}>
      {children}
    </div>
  );
});
