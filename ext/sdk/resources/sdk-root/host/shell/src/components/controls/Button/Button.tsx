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
  icon?: React.ReactNode,
  theme?: ButtonTheme,
  disabled?: boolean,
  onClick?: () => void,
  autofocus?: boolean,
  tabIndex?: number,
}

export const Button = React.memo(function Button(props: ButtonProps) {
  const {
    text = null,
    icon = null,
    theme = 'default',
    disabled = false,
    onClick = noop,
    autofocus = false,
    tabIndex,
  } = props;

  const className = classnames(s.root, s[theme], {
    [s.disabled]: disabled,
    [s.icon]: !!icon,
    [s.text]: !!text,
    [s.autofocus]: autofocus || (typeof tabIndex !== 'undefined'),
  });

  return (
    <button
      className={className}
      onClick={onClick}
      autoFocus={autofocus}
      tabIndex={tabIndex}
    >
      {icon}
      {text}
    </button>
  );
});
