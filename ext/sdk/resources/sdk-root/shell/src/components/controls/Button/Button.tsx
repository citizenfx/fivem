import React from 'react';
import classnames from 'classnames';
import s from './Button.module.scss';


const noop = () => {};

export type ButtonTheme =
  | 'default'
  | 'primary'
  | 'transparent';

export interface ButtonProps {
  text?: string,
  title?: string,
  icon?: React.ReactNode,
  theme?: ButtonTheme,
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
    disabled = false,
    onClick = noop,
    autofocus = false,
    tabIndex,
  } = props;

  const rootClassName = classnames(s.root, s[theme], className, {
    [s.disabled]: disabled,
    [s.icon]: !!icon,
    [s.text]: !!text,
    [s.autofocus]: autofocus || (typeof tabIndex !== 'undefined'),
  });

  return (
    <button
      className={rootClassName}
      onClick={onClick}
      autoFocus={autofocus}
      tabIndex={tabIndex}
      title={title}
    >
      {icon}
      {!!icon && '\u00A0'}
      {text}
    </button>
  );
});
