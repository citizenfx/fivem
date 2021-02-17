import React from 'react';
import classnames from 'classnames';
import s from './Input.module.scss';


export interface InputProps {
  type?: 'text' | 'password' | 'search',
  tabIndex?: number,
  disabled?: boolean,
  autofocus?: boolean,
  pattern?: RegExp,
  className?: string,
  inputClassName?: string,
  label?: string,
  placeholder?: string,
  description?: React.ReactNode,
  value: string,
  onChange: (string) => void,
  onSubmit?: () => void,

  noSpellCheck?: boolean,
}

export const Input = React.memo(function Input(props: InputProps) {
  const {
    label,
    value,
    onChange,
    onSubmit,
    pattern,
    tabIndex,
    noSpellCheck = false,
    autofocus = false,
    disabled = false,
    className = '',
    placeholder = '',
    inputClassName = '',
    description = '',
    type = 'text',
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
      type={type}
      tabIndex={tabIndex}
      autoFocus={autofocus}
      className={inputClassName}
      value={value}
      placeholder={placeholder}
      disabled={disabled}
      onChange={handleChange}
      onKeyDown={handleKeyDown}
      spellCheck={!noSpellCheck}
    />
  );

  const descriptionNode = description
    ? (
      <div className={s.description}>
        {description}
      </div>
    )
    : null;

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
      {descriptionNode}
    </div>
  );
});
