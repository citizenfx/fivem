import React from 'react';
import classnames from 'classnames';
import s from './Input.module.scss';
import { Indicator } from 'fxdk/ui/Indicator/Indicator';


export interface InputProps {
  size?: 'normal' | 'small',
  type?: 'text' | 'password' | 'search' | 'number',
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

  decorator?(): React.ReactNode,

  // for 'range' inputs
  combi?: boolean,
  min?: number | undefined,
  max?: number | undefined,

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
    size = 'normal',
    decorator,
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
    } else if (type === 'number') {
      if (/^[0-9]*$/.test(value)) {
        const intValue = parseInt(value, 10);

        const minValue = typeof min === 'undefined'
          ? Number.MIN_SAFE_INTEGER
          : min;
        const maxValue = typeof max === 'undefined'
          ? Number.MAX_SAFE_INTEGER
          : max;

        if (value === '' || (intValue >= minValue && intValue <= maxValue)) {
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

  const decoratorNode = (showLoader || !!decorator)
    ? (
      <div className={s.decorator}>
        {!!showLoader && <Indicator />}
        {!!decorator && decorator()}
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
        min={min}
        max={max}
      />
      {decoratorNode}
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
    <div className={classnames(s.root, s[size], className)}>
      {contentNode}
      {descriptionNode}
    </div>
  );
});
