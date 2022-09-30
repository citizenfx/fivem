import React from 'react';
import { clsx } from 'cfx/utils/clsx';
import s from './Text.module.scss';
import { ui } from '../ui';

export type TextColor =
  | 'inherit'
  | 'main'
  | 'primary'
  | 'teal'
  | 'success'
  | 'warning'
  | 'error'

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

interface TextPropsBase {
  asDiv?: boolean,
  centered?: boolean,
  truncated?: boolean,

  /**
   * If it'll be a multiline text, use this to add extra spacing between lines
   */
  typographic?: boolean,

  uppercase?: boolean,

  size?: TextSize,
  weight?: TextWeight,

  family?: 'primary' | 'secondary' | 'monospace',

  children?: React.ReactNode,
  className?: string,
}

interface TextPropsFullControl extends TextPropsBase {
  color?: TextColor,
  opacity?: TextOpacity,
}

interface TextPropsColorToken extends TextPropsBase {
  colorToken: string,
}

export type TextProps =
  | TextPropsFullControl
  | TextPropsColorToken;

function getTextColor(props: TextProps): string {
  if ('colorToken' in props) {
    return `var(--color-${props.colorToken})`;
  }

  const {
    color = 'main',
    opacity = 1,
  } = props;

  const variant = color === 'main'
    ? '950'
    : 'pure';

  const colorVarName = variant === 'pure'
    ? `--color-${color}`
    : `--color-${color}-${variant}`;

  return `rgba(var(${colorVarName}), ${TEXT_OPACITY_MAP[opacity] || TEXT_OPACITY_MAP[100]})`;
}

export const Text = React.forwardRef(function Text(props: TextProps, ref: React.Ref<HTMLSpanElement | HTMLDivElement>) {
  const {
    family = 'primary',
    size = 'normal',
    weight = 'normal',

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

  const style: Partial<React.CSSProperties> = {
    color: getTextColor(props),
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
    <Text ref={ref} {...props} asDiv />
  );
});
