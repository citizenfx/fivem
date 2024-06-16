import React from 'react';
import { Link } from 'react-router-dom';

import { clsx } from 'cfx/utils/clsx';
import { noop } from 'cfx/utils/functional';
import { isExternalUrl } from 'cfx/utils/links';

import type { ButtonSize, ButtonTheme } from './Button';

import s from './Button.module.scss';

export interface LinkButtonProps {
  to: string;
  text?: React.ReactNode;
  title?: string;
  icon?: React.ReactNode;
  theme?: ButtonTheme;
  size?: ButtonSize;
  disabled?: boolean;
  onClick?(): void;
  tabIndex?: number;
  className?: string;

  decorator?: React.ReactNode;
}

export const LinkButton = React.forwardRef(function LinkButton(
  props: LinkButtonProps,
  ref: React.Ref<HTMLAnchorElement>,
) {
  const {
    to,
    text = null,
    icon = null,
    title = '',
    className = '',
    theme = 'default',
    size = 'normal',
    disabled = false,
    tabIndex,
    onClick = noop,
    decorator = null,
  } = props;

  const rootClassName = clsx(s.root, s[theme], s[size], className, {
    [s.disabled]: disabled,
    [s.icon]: !!icon && !text,
    [s.text]: !!text,
  });

  const isExternalLink = isExternalUrl(to);

  const Component = isExternalLink
    ? 'a'
    : Link;

  const linkProps: any = isExternalLink
    ? { href: to }
    : { to };

  return (
    <Component
      ref={ref}
      className={rootClassName}
      tabIndex={tabIndex}
      title={title}
      onClickCapture={onClick}
      {...linkProps}
    >
      {!!icon && (
        <span className={s['icon-node']}>{icon}</span>
      )}
      {text}
      {!!decorator && (
        <div className={s.decorator}>{decorator}</div>
      )}
    </Component>
  );
});
