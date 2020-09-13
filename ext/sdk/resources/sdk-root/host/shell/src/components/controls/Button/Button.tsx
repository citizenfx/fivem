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
}

export const Button = React.memo((props: ButtonProps) => {
  const {
    text = null,
    icon = null,
    theme = 'default',
    disabled = false,
    onClick = noop,
  } = props;

  const className = classnames(s.root, s[theme], {
    [s.disabled]: disabled,
    [s.icon]: !!icon,
    [s.text]: !!text,
  });

  return (
    <button
      className={className}
      onClick={onClick}
    >
      {icon}
      {text}
    </button>
  );
});
