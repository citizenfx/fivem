import React from "react";
import { clsx } from "cfx/utils/clsx";
import { useDynamicRef } from "cfx/utils/hooks";
import { InputSize } from "./Input";
import s from './RichInput.module.scss';
import { Button } from "../Button/Button";
import { Icons } from "../Icons";

export interface RichInputProps {
  size?: InputSize,

  value: string,
  onChange(value: string): void,
  onFocus?(): void,
  onBlur?(): void,
  onSelect?(start: number | null, end: number | null, direction: HTMLInputElement['selectionDirection']): void,
  onKeyDown?(event: React.KeyboardEvent<HTMLInputElement>): void,

  rendered: React.ReactNode,

  className?: string,
  placeholder?: string,

  autoFocus?: boolean,
  withClearButton?: boolean,
}

export const RichInput = React.forwardRef((props: RichInputProps, ref: React.RefObject<HTMLDivElement>) => {
  const {
    size = 'normal',
    value,
    onChange,
    onFocus,
    onBlur,
    onSelect,
    onKeyDown,
    className,
    rendered,
    placeholder,
    autoFocus,
    withClearButton = false,
  } = props;

  const inputRef = React.useRef<HTMLInputElement>(null);
  const rendererRef = React.useRef<HTMLDivElement>(null);

  const onChangeRef = useDynamicRef(onChange);
  const onSelectRef = useDynamicRef(onSelect);

  const handleChange = React.useCallback((event: React.ChangeEvent<HTMLInputElement>) => {
    onChangeRef.current(event.target.value);
  }, []);

  const handleClear = React.useCallback(() => {
    onChangeRef.current('');
    inputRef.current?.focus();
  }, []);

  const handleSelect = React.useCallback((event: React.SyntheticEvent<HTMLInputElement>) => {
    if (!onSelectRef.current) {
      return;
    }

    if (!(event.target instanceof HTMLInputElement)) {
      return;
    }

    const $input = event.target;

    onSelectRef.current($input.selectionStart, $input.selectionEnd, $input.selectionDirection);
  }, []);

  const handleScroll = React.useCallback((event: React.WheelEvent<HTMLInputElement>) => {
    if (!rendererRef.current) {
      return;
    }

    rendererRef.current.scrollTop = (event.target as HTMLInputElement).scrollTop;
    rendererRef.current.scrollLeft = (event.target as HTMLInputElement).scrollLeft;
  }, []);

  const rootClassName = clsx(s.root, className, s[size]);

  return (
    <div ref={ref} className={rootClassName}>
      <input
        ref={inputRef}

        autoFocus={autoFocus}

        type="text"
        value={value}
        onChange={handleChange}
        placeholder={placeholder}

        onBlur={onBlur}
        onFocus={onFocus}
        onScroll={handleScroll}
        onSelect={handleSelect}

        onKeyDown={onKeyDown}
      />

      <div ref={rendererRef} className={s.renderer}>
        {rendered}
      </div>

      {(withClearButton && value) && (
        <div className={s.clear}>
          <Button
            size="small"
            icon={Icons.exit}
            onClick={handleClear}
          />
        </div>
      )}
    </div>
  );
});
