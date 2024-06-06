import React from 'react';

import { Indicator } from 'cfx/ui/Indicator/Indicator';
import { clsx } from 'cfx/utils/clsx';
import { getValue } from 'cfx/utils/getValue';
import { useDynamicRef } from 'cfx/utils/hooks';

import s from './Input.module.scss';

export type InputSize = 'small' | 'normal' | 'large';

type TypeSpecific =
  // For normal stringy types
  | { type?: 'text' | 'password' | 'search'; value: string; onChange(value: string): void }
  // Number is kinda a special snowflake
  | {
    type: 'number';
    min?: number | string;
    max?: number | string;
    value: number | string;
    onChange(value: string): void;
  };

export type InputProps = TypeSpecific & {
  fullWidth?: boolean;
  size?: InputSize;
  tabIndex?: number;
  disabled?: boolean;
  autofocus?: boolean;
  pattern?: RegExp;
  className?: string;
  backdropBlur?: boolean;
  inputClassName?: string;
  inputRef?: React.Ref<HTMLInputElement>;
  label?: React.ReactNode;
  placeholder?: string;
  description?: React.ReactNode;

  error?: boolean;

  decorator?: React.ReactNode | (() => React.ReactNode);

  onSubmit?: () => void;
  onKeyDown?(event: React.KeyboardEvent<HTMLInputElement>): void | boolean;

  showLoader?: boolean;
  noSpellCheck?: boolean;
};

export const Input = React.forwardRef(function Input(props: InputProps, ref: React.Ref<HTMLDivElement>) {
  const {
    label,
    value,
    onChange,
    onSubmit,
    onKeyDown = () => false,
    pattern,
    tabIndex,
    error = false,
    fullWidth = false,
    showLoader = false,
    noSpellCheck = false,
    autofocus = false,
    disabled = false,
    className = '',
    placeholder = '',
    inputClassName = '',
    inputRef,
    description = '',
    type = 'text',
    size = 'normal',
    decorator,
    backdropBlur = false,
  } = props;

  const min = (props.type === 'number' && (props.min ?? Number.MIN_SAFE_INTEGER)) || undefined;
  const max = (props.type === 'number' && (props.max ?? Number.MAX_SAFE_INTEGER)) || undefined;
  const controlId = React.useId();

  const onChangeRef = useDynamicRef(onChange);
  const onKeyDownRef = useDynamicRef(onKeyDown);
  const onSubmitRef = useDynamicRef(onSubmit);

  const handleChange = React.useCallback(
    (event: React.ChangeEvent<HTMLInputElement>) => {
      const {
        value,
      } = event.target;

      if (pattern) {
        if (pattern.test(value)) {
          onChangeRef.current(value);
        }

        return;
      }

      return onChangeRef.current(value);
    },
    [pattern],
  );

  const handleKeyDown = React.useCallback((event: React.KeyboardEvent<HTMLInputElement>) => {
    if (onKeyDownRef.current(event)) {
      return;
    }

    if (onSubmitRef.current && event.key === 'Enter') {
      onSubmitRef.current();
    }
  }, []);

  const decoratorNode = showLoader || !!decorator
    ? (
      <div className={s.decorator}>
        {!!showLoader && (
          <Indicator />
        )}
        {!!decorator && getValue(decorator)}
      </div>
      )
    : null;

  const inputNode = (
    <div className={s.input}>
      <input
        id={controlId}
        ref={inputRef}
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
      <div className={s.description}>{description}</div>
      )
    : null;

  const contentNode = label
    ? (
      <label htmlFor={controlId}>
        {label}
        {inputNode}
      </label>
      )
    : (
        inputNode
      );

  const rootClassName = clsx(s.root, s[size], className, {
    [s['full-width']]: fullWidth,
    [s.error]: error,
    [s.disabled]: disabled,
    [s['backdrop-blur']]: backdropBlur,
  });

  return (
    <div ref={ref} className={rootClassName}>
      {contentNode}
      {descriptionNode}
    </div>
  );
});
