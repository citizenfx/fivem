import * as RadSelect from '@radix-ui/react-select';
import React from 'react';
import { BsChevronDown, BsChevronUp } from 'react-icons/bs';

import { clsx } from 'cfx/utils/clsx';

import { Icons } from '../Icons';

import s from './Select.module.scss';

export interface SelectOption<T> {
  value: T;
  label: React.ReactNode;
}

export interface SelectProps<T = string> {
  options: SelectOption<T>[];
  value: T;
  onChange: (value: T) => void;

  className?: string;
  disabled?: boolean;
  size?: 'normal' | 'small';
}

export const Select = React.forwardRef(function Select(props: SelectProps) {
  const {
    value,
    options,
    onChange,

    size = 'normal',
    disabled = false,
    className,
  } = props;

  const [open, setOpen] = React.useState(false);

  const triggerClassName = clsx(s.radTrigger, s[size], className, {
    [s.disabled]: disabled,
  });

  return (
    <RadSelect.Root value={value} onValueChange={onChange} onOpenChange={setOpen} open={open}>
      <RadSelect.Trigger disabled={disabled} className={triggerClassName}>
        <RadSelect.Value />
        <RadSelect.Icon>
          <BsChevronDown />
        </RadSelect.Icon>
      </RadSelect.Trigger>

      <RadSelect.Portal>
        <RadSelect.Content className={s.radContent}>
          <RadSelect.ScrollUpButton className={s.radArrow}>
            <BsChevronUp />
          </RadSelect.ScrollUpButton>

          <RadSelect.Viewport>
            {options.map((option) => (
              <RadSelect.Item key={option.value} value={option.value} className={s.radOption}>
                <RadSelect.ItemText>{option.label}</RadSelect.ItemText>
                <RadSelect.ItemIndicator className={s.radIcon}>{Icons.checkmark}</RadSelect.ItemIndicator>
              </RadSelect.Item>
            ))}
          </RadSelect.Viewport>

          <RadSelect.ScrollDownButton className={s.radArrow}>
            <BsChevronDown />
          </RadSelect.ScrollDownButton>
        </RadSelect.Content>
      </RadSelect.Portal>
    </RadSelect.Root>
  );
});

export const Select2 = function Select2<T extends string | number>(props: SelectProps<T>) {
  const {
    value,
    options,
    onChange,

    size = 'normal',
    disabled = false,
    className,
  } = props;

  const handleChange = React.useCallback(({
    target,
  }) => onChange(target.value), [onChange]);

  const rootClassName = clsx(s.root, s[size], className, {
    [s.disabled]: disabled,
  });

  return (
    <select disabled={disabled} className={rootClassName} value={value} onChange={handleChange}>
      {options.map((option) => (
        <option key={option.value} value={option.value}>
          {option.label}
        </option>
      ))}
    </select>
  );
};
