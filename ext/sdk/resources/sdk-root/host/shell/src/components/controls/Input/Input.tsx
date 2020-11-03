import React from 'react';
import classnames from 'classnames';
import s from './Input.module.scss';


export interface InputProps {
  tabIndex?: number,
  disabled?: boolean,
  autofocus?: boolean,
  pattern?: RegExp,
  className?: string,
  inputClassName?: string,
  label?: string,
  placeholder?: string,
  value: string,
  onChange: (string) => void,
  onSubmit?: () => void,
}

export const Input = React.memo(function Input(props: InputProps) {
  const {
    label,
    value,
    onChange,
    onSubmit,
    pattern,
    tabIndex,
    autofocus = false,
    disabled = false,
    className = '',
    placeholder = '',
    inputClassName = '',
  } = props;

  const handleChange = React.useCallback((event) => {
    const { value } = event.target;

    if (pattern && value) {
      if (pattern.test(value)) {
        onChange(value);
      }
    } else {
      onChange(value);
    }
  }, [onChange, pattern]);

  const handleKeyDown = React.useCallback((event: React.KeyboardEvent<HTMLInputElement>) => {
    if (event.key === 'Enter' && onSubmit) {
      onSubmit();
    }
  }, [onSubmit]);

  const input = (
    <input
      tabIndex={tabIndex}
      autoFocus={autofocus}
      className={inputClassName}
      value={value}
      placeholder={placeholder}
      disabled={disabled}
      onChange={handleChange}
      onKeyDown={handleKeyDown}
    />
  );

  const content = label
    ? (
      <label>
        {label}
        {input}
      </label>
    )
    : input;

  return (
    <div className={classnames(s.root, className)}>
      {content}
    </div>
  );
});
