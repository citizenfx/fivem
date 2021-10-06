import * as React from 'react';
import classnames from 'classnames';
import s from './TabSelector.module.scss';

export interface TabItem {
  value: string,
  label: string,
  icon?: JSX.Element,
}

export interface TabSelectorProps {
  value: string,
  items: TabItem[],
  onChange: (value: string) => void,
  className?: string,
  vertical?: boolean,
}

export const TabSelector = React.memo(function TabSelector(props: TabSelectorProps) {
  const {
    value,
    items,
    onChange,
    className = '',
    vertical = false,
  } = props;

  const rootClassName = classnames(s.root, className, {
    [s.vertical]: vertical,
  });

  const itemNodes = items.map((item) => {
    const handleClick = () => onChange(item.value);

    const itemClassName = classnames(s.item, {
      [s.active]: item.value === value,
    });

    return (
      <div
        key={item.value}
        className={itemClassName}
        onClick={handleClick}
      >
        {item.icon}
        {item.label}
      </div>
    );
  });

  return (
    <div className={rootClassName}>
      {itemNodes}
      <div className={s.extender}/>
    </div>
  );
});
