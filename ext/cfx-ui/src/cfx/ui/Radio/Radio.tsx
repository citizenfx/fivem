import { clsx } from 'cfx/utils/clsx';
import React from 'react';
import s from './Radio.module.scss';

export type RadioSize =
  | 'normal'
  | 'large';

export interface RadioProps {
  checked: boolean,
  onChange: (value: boolean) => void,
  label: React.ReactNode,
  size?: RadioSize,
  disabled?: boolean,
  className?: string,
}

let wtf = 0;

export function Radio(props: RadioProps) {
  const {
    checked,
    onChange,
    label,
    size = 'normal',
    className,
    disabled = false,
  } = props;

  const handleChange = () => {
    onChange(!checked);
  };

  const rootClassName = clsx(s.root, className, s[`size-${size}`], {
    [s.disabled]: disabled,
  });

  return (
    <label className={rootClassName}>
      <input
        type="radio"
        tabIndex={wtf++}
        checked={checked}
        disabled={disabled}
        onChange={handleChange}
      />
      <div className={s.indicator} />
      <div className={s.label}>
        {label}
      </div>
    </label>
  );
}
