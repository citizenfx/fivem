import React from 'react';

import { clsx } from 'cfx/utils/clsx';
import { noop } from 'cfx/utils/functional';
import { getValue, ValueOrGetter } from 'cfx/utils/getValue';

import { Interactive } from '../Interactive/Interactive';

import s from './NavList.module.scss';

export interface NavListItem {
  id: string;
  icon?: ValueOrGetter<React.ReactNode>;
  label: ValueOrGetter<React.ReactNode>;
}

export interface NavListProps {
  items: NavListItem[];
  activeItemId: string;
  onActivate?(id: string): void;
}

export const NavList = React.forwardRef(function NavList(props: NavListProps, ref: React.Ref<HTMLDivElement>) {
  const {
    items,
    activeItemId,
    onActivate = noop,
  } = props;

  const itemNodes = items.map((item) => (
    <Interactive
      key={item.id}
      className={clsx(s.item, { [s.active]: item.id === activeItemId })}
      onClick={() => onActivate(item.id)}
    >
      {!!item.icon && (
        <div className={s.icon}>{getValue(item.icon)}</div>
      )}

      <div className={s.label}>{getValue(item.label)}</div>
    </Interactive>
  ));

  return (
    <div ref={ref} className={s.root}>
      {itemNodes}
    </div>
  );
});
