import React from 'react';
import { noop, returnTrue } from 'fxdk/base/functional';

export interface InPlaceProps {
  type?: string,
  value: string,
  onChange(value: string): void,
  placeholder?: string,
  className?: string,
  validate?(value: string): boolean,
  onIntermediateChange?(value: string): void,
}

export function InPlaceInput(props: InPlaceProps) {
  const {
    type = 'text',
    value,
    onChange,
    placeholder,
    className = '',
    validate = returnTrue,
    onIntermediateChange = noop,
  } = props;

  const [intermediateValue, setIntermediateValue] = React.useState(value);

  const handleKeyDown = React.useCallback((event: React.KeyboardEvent<HTMLInputElement>) => {
    if (event.key === 'Enter') {
      event.stopPropagation();
      return onChange(intermediateValue);
    }

    if (event.key === 'Escape') {
      event.stopPropagation();
      setIntermediateValue(value);
      return onChange(value);
    }
  }, [onChange, intermediateValue, value]);

  const handleBlur = React.useCallback(() => {
    setIntermediateValue(value);
    onChange(value);
  }, [onChange, value])

  const handleChange = React.useCallback(({ target: { value } }) => {
    if (!value) {
      setIntermediateValue('');
      onIntermediateChange('');
    } else if (validate(value)) {
      setIntermediateValue(value);
      onIntermediateChange(value);
    }
  }, [setIntermediateValue, validate, onIntermediateChange]);

  return (
    <input
      autoFocus
      type={type}
      value={intermediateValue}
      onChange={handleChange}
      placeholder={placeholder}
      onBlur={handleBlur}
      onKeyDown={handleKeyDown}
      className={className}
    />
  );
}
