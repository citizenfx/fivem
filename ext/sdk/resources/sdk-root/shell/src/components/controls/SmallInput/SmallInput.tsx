import React from 'react';
import classnames from 'classnames';
import s from './SmallInput.module.scss';
import { noop } from 'utils/noop';

export type SmallInputProps<T> = React.HTMLProps<HTMLInputElement> & {
  onChange?: (value: T) => void,
}

export const SmallInput = function SmallInput<T>(props: SmallInputProps<T>) {
  const { onChange = noop } = props;

  const handleChange = React.useCallback(({ target: { value } }) => onChange(value), [onChange]);

  return (
    <input
      {...props}
      className={classnames(s.root, props.className)}
      onChange={handleChange}
    />
  );
};
