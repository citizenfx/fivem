import React from 'react';
import classnames from 'classnames';
import s from './Input.module.scss';
import { Indicator } from 'components/Indicator/Indicator';


export interface InputProps {
  type?: 'text' | 'password' | 'search' | 'range',
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

  // for 'range' inputs
  combi?: boolean,
  min?: string,
  max?: string,

  onChange: (value: string) => void,
  onSubmit?: () => void,
  onKeyDown?(event: React.KeyboardEvent<HTMLInputElement>): void | boolean,

  showLoader?: boolean,
  noSpellCheck?: boolean,
}

export const Input = React.memo(function Input(props: InputProps) {
  const {
    label,
    value,
    onChange,
    onSubmit,
    onKeyDown = () => false,
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
    combi,
    min,
    max
  } = props;

  const handleChange = React.useCallback((event) => {
    const { value } = event.target;

    if (pattern && value) {
      if (pattern.test(value)) {
        onChange(value);
      }
    } else if (type === 'range') {
      if (/^[0-9]*$/.test(value)) {
        const intValue = value | 0;
        if (value === '' || (intValue >= (parseInt(min, 10)) && intValue <= (parseInt(max, 10)))) {
          onChange(value);
        }
      }
    } else {
      onChange(value);
    }
  }, [onChange, pattern]);

  const handleKeyDown = React.useCallback((event: React.KeyboardEvent<HTMLInputElement>) => {
    if (onKeyDown(event)) {
      return;
    }

    if (event.key === 'Enter' && onSubmit) {
      onSubmit();
    }
  }, [onSubmit, onKeyDown]);

  const loaderNode = showLoader
    ? (
      <div className={s.loader}>
        <Indicator />
      </div>
    )
    : null;

  const curNode = (!combi && type === 'range')
    ? (
      <span>{value}</span>
    )
    : null;

  const textNode = (combi && type === 'range')
    ? (
        <input
        type='text'
        className={inputClassName}
        value={value}
        disabled={disabled}
        onChange={handleChange}
        onKeyDown={handleKeyDown}
        spellCheck={false}
      />
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
        min={min}
        max={max}
      />
      {curNode}
      {textNode}
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
