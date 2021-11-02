import React from 'react';
import classnames from 'classnames';
import s from './RangeInput.module.scss';

export interface RangeInputProps {
  value: number,
  onChange: (value: number) => void,
  min?: number | undefined,
  max?: number | undefined,
  step?: number,
  size?: 'normal' | 'small',
  className?: string,
  disabled?: boolean,
}

export function RangeInput(props: RangeInputProps) {
  const {
    value,
    onChange,
    className,
    disabled,

    min = Number.MIN_SAFE_INTEGER,
    max = Number.MAX_SAFE_INTEGER,
    step = 1,
    size = 'normal',
  } = props;

  const handleChange = React.useCallback(({ target: { value: valueString } }) => {
    const valueNumber = Number(valueString);

    if (valueNumber < min) {
      return onChange(min);
    }
    if (valueNumber > max) {
      return onChange(max);
    }

    onChange(valueNumber);
  }, [onChange, min, max]);

  const rootClassName = classnames(s.root, s[size], className, {
    [s.disabled]: disabled,
  });

  return (
    <div className={rootClassName}>
      {(min !== Number.MIN_SAFE_INTEGER) && (
        <div className={s.min} onClick={() => onChange(min)}>
          {min}
        </div>
      )}
      <input
        type="range"
        value={value}
        onChange={handleChange}
        min={min}
        max={max}
        step={step}
      />
      {(max !== Number.MAX_SAFE_INTEGER) && (
        <div className={s.max} onClick={() => onChange(max)}>
          {max}
        </div>
      )}
    </div>
  );
}
