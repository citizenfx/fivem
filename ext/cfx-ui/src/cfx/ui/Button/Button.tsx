import React from 'react';
import { clsx } from 'cfx/utils/clsx';
import { noop } from 'cfx/utils/functional';
import { useLinkClickHandler } from 'react-router-dom';
import s from './Button.module.scss';


export type ButtonTheme =
  | 'default'
  | 'default-blurred'
  | 'primary'
  | 'transparent';

export type ButtonSize =
  | 'normal'
  | 'small'
  | 'large';

export interface ButtonProps {
  /**
   * Will turn button into link
   */
  to?: string,

  text?: React.ReactNode,
  title?: string,
  icon?: React.ReactNode,
  theme?: ButtonTheme,
  size?: ButtonSize,
  straightCorners?: boolean,
  disabled?: boolean,
  autofocus?: boolean,
  tabIndex?: number,
  className?: string,

  onClick?: (event: React.MouseEvent<HTMLButtonElement>) => void,
  onMouseDown?: (event: React.MouseEvent<HTMLButtonElement>) => void,
  onMouseUp?: (event: React.MouseEvent<HTMLButtonElement>) => void,

  decorator?: React.ReactNode,
}

export const Button = React.forwardRef(function Button(props: ButtonProps, ref: React.Ref<HTMLButtonElement>) {
  const {
    to,
    text = null,
    icon = null,
    title = '',
    className = '',
    theme = 'default',
    size = 'normal',
    disabled = false,
    straightCorners = false,
    onClick = noop,
    onMouseDown = noop,
    onMouseUp = noop,
    autofocus = false,
    tabIndex,
    decorator = null,
  } = props;

  const rootClassName = clsx(s.root, s[theme], s[size], className, {
    [s.disabled]: disabled,
    [s.icon]: !!icon && (text === null || typeof text === 'undefined'),
    [s.text]: !!text,
    [s.autofocus]: autofocus || (typeof tabIndex !== 'undefined'),
    [s['straight-borders']]: straightCorners,
  });

  const linkClickHandler = to
    ? useLinkClickHandler(to)
    : noop;

  const handleClick = (event: React.MouseEvent<HTMLButtonElement>) => {
    if (disabled) {
      return;
    }

    onClick(event);

    if (event.defaultPrevented) {
      return;
    }

    linkClickHandler(event as any);
  };

  return (
    <button
      ref={ref as any}
      disabled={disabled}
      className={rootClassName}
      onClick={handleClick}
      onMouseDown={onMouseDown}
      onMouseUp={onMouseUp}
      autoFocus={autofocus}
      tabIndex={tabIndex}
      title={title}
    >
      {!!icon && (
        <span className={s['icon-node']}>{icon}</span>
      )}

      {text}

      {!!decorator && (
        <div className={s.decorator}>
          {decorator}
        </div>
      )}
    </button>
  );
});
