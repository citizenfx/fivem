import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import { TextOpacity, TextProps } from './Text.types';
import { ui } from '../ui';

import s from './Text.module.scss';

export const TEXT_OPACITY_MAP: Record<TextOpacity, number | string> = {
  '0': 0,
  '25': 'var(--text-opacity-25)',
  '50': 'var(--text-opacity-50)',
  '75': 'var(--text-opacity-75)',
  '100': 1,
};

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
    userSelectable = false,

    children,
    className,
  } = props;

  const rootClassName = clsx(s.root, className, {
    [s.block]: asDiv,
    [s.centered]: centered,
    [s.truncated]: truncated,
    [s.typographic]: typographic,
    [ui.cls.userSelectableText]: userSelectable,
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

export const TextBlock = React.forwardRef(function TextBlock(
  props: TextProps & { asDiv?: undefined },
  ref: React.Ref<HTMLDivElement>,
) {
  return (
    <Text ref={ref} {...props} asDiv />
  );
});
