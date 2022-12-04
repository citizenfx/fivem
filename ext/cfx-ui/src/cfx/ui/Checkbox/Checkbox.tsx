import React from 'react';
import { clsx } from 'cfx/utils/clsx';
import s from './Checkbox.module.scss';

export type CheckboxSize =
  | 'normal'
  | 'large';

export interface CheckboxProps {
  value: boolean,
  onChange: (value: boolean) => void,
  size?: CheckboxSize,
  label?: React.ReactNode,
  disabled?: boolean,
  className?: string,
}

export const Checkbox = React.memo(function Checkbox(props: CheckboxProps) {
  const {
    value,
    onChange,
    label,
    size = 'normal',
    disabled = false,
    className,
  } = props;

  const handleChange = React.useCallback(() => {
    onChange(!value);
  }, [value, onChange]);

  const rootClassName = clsx(s.root, className, s[`size-${size}`], {
    [s.disabled]: disabled,
    [s['with-label']]: !!label,
  });

  return (
    <label className={rootClassName}>
      <input
        type="checkbox"
        checked={value}
        disabled={disabled}
        onChange={handleChange}
      />
      <div className={s.indicator}/>
      <div className={s.label}>
        {label}
      </div>
    </label>
  );
});
