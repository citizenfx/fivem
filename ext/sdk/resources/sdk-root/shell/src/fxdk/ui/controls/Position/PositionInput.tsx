import React from 'react';
import { limitPrecision } from 'shared/math';
import s from './Position.module.scss';

export interface PositionInputProps {
  value: number,
  label: string,
  onChange(num: number): void,
}

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

export const PositionInput = React.memo(function PositionInput(props: PositionInputProps) {
  const { value, label, onChange } = props;

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
    <div className={s.input}>
      <div>
        {label}
      </div>
      <input
        type="text"
        value={limitPrecision(value)}
        onChange={({ target: { value } }) => onChange(parseFloat(value))}
        onKeyDown={handleKeyDown as any}
        onKeyUp={handleKeyUp as any}
      />
    </div>
  );
});
