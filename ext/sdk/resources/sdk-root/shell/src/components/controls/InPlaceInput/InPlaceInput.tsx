import React from 'react';

export interface InPlaceProps {
  type?: string,
  value: string,
  onChange: (value: string) => void,
  placeholder?: string,
}

export function InPlaceInput(props: InPlaceProps) {
  const {
    type = 'text',
    value,
    onChange,
    placeholder,
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

  return (
    <input
      autoFocus
      type={type}
      value={intermediateValue}
      onChange={({ target }) => setIntermediateValue(target.value)}
      placeholder={placeholder}
      onBlur={handleBlur}
      onKeyDown={handleKeyDown}
    />
  );
}
