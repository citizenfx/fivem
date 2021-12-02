import * as React from 'react';
import classnames from 'classnames';
import s from './Select.module.scss';

export interface SelectOption<T> {
  value: T,
  title: React.ReactNode,
}

export interface SelectProps<T> {
  options: SelectOption<T>[],
  value: T,
  onChange: (value: T) => void,

  className?: string,
  disabled?: boolean,
  size?: 'normal' | 'small',
}

export const Select = function Select<T extends string | number>(props: SelectProps<T>) {
  const {
    value,
    options,
    onChange,

    size = 'normal',
    disabled = false,
    className,
  } = props;

  const handleChange = React.useCallback(({ target }) => onChange(target.value), [onChange]);

  const rootClassName = classnames(s.root, s[size], className, {
    [s.disabled]: disabled,
  });

  return (
    <select
      disabled={disabled}
      className={rootClassName}
      value={value}
      onChange={handleChange}
    >
      {options.map((option) => (
        <option key={option.value} value={option.value}>
          {option.title}
        </option>
      ))}
    </select>
  );
};
