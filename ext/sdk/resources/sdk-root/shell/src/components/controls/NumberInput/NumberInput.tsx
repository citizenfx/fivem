import React from 'react';
import classnames from 'classnames';
import { limitPrecision } from 'shared/math';
import s from './NumberInput.module.scss';

const DEFAULT_MODIFIERS_STATE = {
  slow: false,
  fast: false,
};
type ModifiersState = typeof DEFAULT_MODIFIERS_STATE;

function updateValue(value: number, inc: number, modifiers: ModifiersState) {
  if (modifiers.fast) {
    return value + inc * 5;
  }

  if (modifiers.slow) {
    return value + inc * 0.01;
  }

  return value + inc;
}

export interface NumberInputProps {
  label: string,
  value: number,
  onChange(value: number): void,

  className?: string,
  unlimitedPrecision?: boolean,
}

export function NumberInput(props: NumberInputProps) {
  const {
    label,
    value,
    onChange,

    className,
    unlimitedPrecision = false,
  } = props;

  const valueNumber = unlimitedPrecision
    ? value
    : limitPrecision(value);

  const modifiersRef = React.useRef(DEFAULT_MODIFIERS_STATE);

  const handleKeyDown = (event: KeyboardEvent) => {
    switch (event.key) {
      case 'ArrowUp': onChange(updateValue(value, 1, modifiersRef.current)); break;
      case 'ArrowDown': onChange(updateValue(value, -1, modifiersRef.current)); break;

      case 'Alt': modifiersRef.current.slow = true; break;
      case 'Shift': modifiersRef.current.fast = true; break;

      default: return;
    }

    event.preventDefault();
    event.stopPropagation();
  };

  const handleKeyUp = (event: KeyboardEvent) => {
    switch (event.key) {
      case 'Alt': modifiersRef.current.slow = false; break;
      case 'Shift': modifiersRef.current.fast = false; break;

      default: return;
    }

    event.preventDefault();
    event.stopPropagation();
  };

  return (
    <div className={classnames(s.root, className)}>
      <div className={s.label}>
        {label}
      </div>
      <input
        type="text"
        value={valueNumber}
        onChange={({ target: { value } }) => onChange(parseFloat(value))}
        onKeyDown={handleKeyDown as any}
        onKeyUp={handleKeyUp as any}
      />
    </div>
  );
}
