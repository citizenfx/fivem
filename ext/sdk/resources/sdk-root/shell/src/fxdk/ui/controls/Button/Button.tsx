import React from 'react';
import classnames from 'classnames';
import s from './Button.module.scss';


const noop = () => {};

export type ButtonTheme =
  | 'default'
  | 'primary'
  | 'transparent';

export type ButtonSize =
  | 'normal'
  | 'small';

export interface ButtonProps {
  text?: string,
  title?: string,
  icon?: React.ReactNode,
  theme?: ButtonTheme,
  size?: ButtonSize,
  disabled?: boolean,
  onClick?: () => void,
  autofocus?: boolean,
  tabIndex?: number,
  className?: string,
}

export const Button = React.memo(function Button(props: ButtonProps) {
  const {
    text = null,
    icon = null,
    title = '',
    className = '',
    theme = 'default',
    size = 'normal',
    disabled = false,
    onClick = noop,
    autofocus = false,
    tabIndex,
  } = props;

  const rootClassName = classnames(s.root, s[theme], s[size], className, {
    [s.disabled]: disabled,
    [s.icon]: !!icon && !text,
    [s.text]: !!text,
    [s.autofocus]: autofocus || (typeof tabIndex !== 'undefined'),
  });

  return (
    <button
      disabled={disabled}
      className={rootClassName}
      onClick={disabled ? noop : onClick}
      autoFocus={autofocus}
      tabIndex={tabIndex}
      title={title}
    >
      {!!icon && (
        <span className={s['icon-node']}>{icon}</span>
      )}
      {text}
    </button>
  );
});
