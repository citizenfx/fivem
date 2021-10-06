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
}

export const Select = function Select<T extends string | number>(props: SelectProps<T>) {
  const {
    value,
    options,
    onChange,

    disabled = false,
    className,
  } = props;

  const handleChange = React.useCallback(({ target }) => onChange(target.value), [onChange]);

  const rootClassName = classnames(s.root, className, {
    [s.disabled]: disabled,
  });

  return (
    <select
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
