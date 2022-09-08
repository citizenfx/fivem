import React from 'react';
import { clsx } from 'cfx/utils/clsx';
import s from './Text.module.scss';

export type TextColor =
  | 'inherit'
  | 'main'
  | 'primary'
  | 'teal'
  | 'success'
  | 'warning'
  | 'error'

export type TextVariant =
  | 'pure'
  | '50'
  | '100'
  | '200'
  | '300'
  | '400'
  | '500'
  | '600'
  | '700'
  | '800'
  | '900'
  | '950'

export type TextSize =
  | 'xsmall'
  | 'small'
  | 'normal'
  | 'large'
  | 'xlarge'
  | 'xxlarge'

export type TextWeight =
  | 'thin'
  | 'normal'
  | 'bold'
  | 'bolder'

export type TextOpacity =
  | '0'
  | '25'
  | '50'
  | '75'
  | '100'

export const TEXT_OPACITY_MAP: Record<TextOpacity, number | string> = {
  0: 0,
  25: 'var(--text-opacity-25)',
  50: 'var(--text-opacity-50)',
  75: 'var(--text-opacity-75)',
  100: 1,
};

export interface TextProps {
  asDiv?: boolean,
  centered?: boolean,
  truncated?: boolean,

  /**
   * If it'll be a multiline text, use this to add extra spacing between lines
   */
  typographic?: boolean,

  uppercase?: boolean,

  size?: TextSize,
  color?: TextColor,
  variant?: TextVariant,
  weight?: TextWeight,

  family?: 'primary' | 'secondary' | 'monospace',

  opacity?: TextOpacity,

  children?: React.ReactNode,
  className?: string,
}

export const Text = React.forwardRef(function Text(props: TextProps, ref: React.Ref<HTMLSpanElement | HTMLDivElement>) {
  const {
    family = 'primary',
    color = 'main',
    variant = color === 'main'
      ? '950'
      : 'pure',
    size = 'normal',
    weight = 'normal',
    opacity = 1,

    asDiv = false,
    centered = false,
    truncated = false,
    typographic = false,
    uppercase = false,

    children,
    className,
  } = props;

  const rootClassName = clsx(s.root, className, {
    [s.block]: asDiv,
    [s.centered]: centered,
    [s.truncated]: truncated,
    [s.typographic]: typographic,
  });

  const colorVarName = variant === 'pure'
    ? `--color-${color}`
    : `--color-${color}-${variant}`;

  const style: Partial<React.CSSProperties> = {
    color: `rgba(var(${colorVarName}), ${TEXT_OPACITY_MAP[opacity] || TEXT_OPACITY_MAP[100]})`,
    fontFamily: `var(--font-family-${family})`,
    fontSize: `var(--font-size-${size})`,
    fontWeight: `var(--font-weight-${weight})`,
  };

  if (uppercase) {
    style.textTransform = 'uppercase';
  }

  const Component = asDiv
    ? 'div'
    : 'span';

  return (
    <Component ref={ref as any} dir="auto" className={rootClassName} style={style}>
      {children}
    </Component>
  );
});

export const TextBlock = React.forwardRef((props: TextProps & { asDiv?: undefined }, ref: React.Ref<HTMLDivElement>) => {
  return (
    <Text ref={ref} {...props} asDiv/>
  );
});
