import React from "react";
import { clsx } from "cfx/utils/clsx";
import s from './Textarea.module.scss';

export interface TextareaProps {
  value: string,
  onChange: (value: string) => void,

  label?: React.ReactNode,
  placeholder?: string,
  rows?: number,
  disabled?: boolean,
  resize?: 'none' | 'vertical' | 'horizontal' | 'both',
  autofocus?: boolean,

  className?: string,
  backdropBlur?: boolean,
}

export function Textarea(props: TextareaProps) {
  const {
    value,
    onChange,

    label,
    placeholder,
    rows,
    resize = 'none',
    disabled = false,
    autofocus,

    className,
    backdropBlur = false,
  } = props;

  const handleChange = React.useCallback((event: React.ChangeEvent<HTMLTextAreaElement>) => {
    onChange(event.target.value);
  }, [onChange]);

  const textareaNode = (
    <textarea
      autoFocus={autofocus}
      className={s[`resize-${resize}`]}
      rows={rows}
      value={value}
      disabled={disabled}
      onChange={handleChange}
      placeholder={placeholder}
    ></textarea>
  );

  const rootClassName = clsx(s.root, className, {
    [s.disabled]: disabled,
    [s['backdrop-blur']]: backdropBlur,
  });

  return (
    <div className={rootClassName}>
      {
        label
          ? (
            <label>
              {label}
              {textareaNode}
            </label>
          )
          : textareaNode
      }
    </div>
  );
}
