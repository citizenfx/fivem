import React from 'react';
import classnames from 'classnames';
import s from './Input.module.scss';
import { Indicator } from 'components/Indicator/Indicator';


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

  showLoader?: boolean,
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
    showLoader = false,
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

  const loaderNode = showLoader
    ? (
      <div className={s.loader}>
        <Indicator />
      </div>
    )
    : null;

  const inputNode = (
    <div className={s.input}>
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
      {loaderNode}
    </div>
  );

  const descriptionNode = description
    ? (
      <div className={s.description}>
        {description}
      </div>
    )
    : null;

  const contentNode = label
    ? (
      <label>
        {label}
        {inputNode}
      </label>
    )
    : inputNode;

  return (
    <div className={classnames(s.root, className)}>
      {contentNode}
      {descriptionNode}
    </div>
  );
});
