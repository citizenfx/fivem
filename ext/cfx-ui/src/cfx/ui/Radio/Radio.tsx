import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import s from './Radio.module.scss';

export type RadioSize = 'normal' | 'large';

export interface RadioProps {
  checked: boolean;
  onChange: (value: boolean) => void;
  label: React.ReactNode;
  size?: RadioSize;
  disabled?: boolean;
  className?: string;
}

let wtf = 0;

export function Radio(props: RadioProps) {
  const {
    checked, onChange, label, size = 'normal', className, disabled = false,
  } = props;

  const handleChange = () => {
    onChange(!checked);
  };

  const controlId = React.useId();

  const rootClassName = clsx(s.root, className, s[`size-${size}`], { [s.disabled]: disabled });

  return (
    <label className={rootClassName} htmlFor={controlId}>
      <input
        type="radio"
        tabIndex={wtf++}
        checked={checked}
        disabled={disabled}
        onChange={handleChange}
        id={controlId}
      />
      <div className={s.indicator} />
      <div className={s.label}>{label}</div>
    </label>
  );
}
