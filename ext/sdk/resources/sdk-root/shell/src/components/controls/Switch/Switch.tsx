import * as React from 'react';
import classnames from 'classnames';
import s from './Switch.module.scss';


const getDefaultOptionDescription = (value: string, options: SwitchOption[]): string => {
  return options.find((option) => option.value === value)?.description || '';
};

export interface SwitchOption {
  value: string,
  label: string,
  icon?: React.ReactNode,
  description?: string,
}

export interface SwitchProps {
  value: string,
  options: SwitchOption[],
  onChange: <T extends string>(value: T) => void,
  className?: string,
  disabled?: boolean,
}

export const Switch = React.memo(function Switch(props: SwitchProps) {
  const {
    value,
    options,
    onChange,
    className = '',
    disabled = false,
  } = props;

  const [optionDescription, setOptionDescription] = React.useState(getDefaultOptionDescription(value, options));

  const optionsNodes = React.useMemo(() => options.map((option, tabIndex) => {
    const handleClick = () => {
      onChange(option.value);
    };

    const handleMouseEnter = () => {
      setOptionDescription(option.description || '');
    };

    const handleMouseLeave = () => {
      setOptionDescription(getDefaultOptionDescription(value, options));
    };

    const optionClassName = classnames(s.option, {
      [s.active]: option.value === value,
    });

    return (
      <div
        key={option.value}
        className={optionClassName}
        tabIndex={tabIndex}
        onClick={handleClick}
        onMouseEnter={handleMouseEnter}
        onMouseLeave={handleMouseLeave}
      >
        {option.icon || null}

        {option.label}
      </div>
    );
  }), [value, options, onChange, setOptionDescription]);

  const rootClassName = classnames(s.root, className, {
    [s.disabled]: disabled,
    [s.descripted]: optionDescription,
  });

  return (
    <div className={rootClassName}>
      {optionsNodes}

      {optionDescription && (
        <div className={s.description}>
          {optionDescription}
        </div>
      )}
    </div>
  );
});
