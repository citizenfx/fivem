import React from "react";
import { clsx } from "cfx/utils/clsx";
import s from './Tabular.module.scss';

export interface TabularItem {
  id: string,

  label: React.ReactNode,
  icon?: React.ReactNode,

  disabled?: boolean,
}

export interface TabularProps {
  items: TabularItem[],

  activeItem: string,
  onActivate(id: string): void,
}
export function Tabular(props: TabularProps) {
  const {
    items,
    activeItem,
    onActivate,
  } = props;

  const nodes = items.map((item) => (
    <div
      key={item.id}
      onClick={() => onActivate(item.id)}
      className={clsx(s.item, { [s.active]: item.id === activeItem, [s.disabled]: !!item.disabled })}
    >
      {item.icon && (
        <div className={s.icon}>
          {item.icon}
        </div>
      )}

      <div className={s.label}>
        {item.label}
      </div>

      <div className={s.content}>
        {item.icon && (
          <div className={s.icon}>
            {item.icon}
          </div>
        )}

        <div className={s.label}>
          {item.label}
        </div>
      </div>

      <div className={s.decorator} />
    </div>
  ));

  return (
    <div className={s.root}>
      {nodes}
    </div>
  );
}
