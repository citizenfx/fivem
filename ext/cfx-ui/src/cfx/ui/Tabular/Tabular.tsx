import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import { Interactive } from '../Interactive/Interactive';

import s from './Tabular.module.scss';

export interface TabularItem {
  id: string;

  label?: React.ReactNode;
  icon?: React.ReactNode;

  disabled?: boolean;
}

export interface TabularProps {
  items: TabularItem[];

  activeItem: string;
  onActivate(id: string): void;
}
export function Tabular(props: TabularProps) {
  const {
    items,
    activeItem,
    onActivate,
  } = props;

  const nodes = items.map((item) => (
    <Tabular.Item
      key={item.id}
      icon={item.icon}
      label={item.label}
      active={item.id === activeItem}
      onClick={() => onActivate(item.id)}
    />
  ));

  return (
    <Tabular.Root>{nodes}</Tabular.Root>
  );
}

export type TabularRootProps = {
  size?: 'normal' | 'large';
  children?: React.ReactNode;
  className?: string;
};

Tabular.Root = function TabularRoot(props: TabularRootProps) {
  const {
    children,
    size = 'normal',
    className,
  } = props;

  const rootClassName = clsx(s.root, s[`size-${size}`], className);

  return (
    <div className={rootClassName}>{children}</div>
  );
};

export type TabularItemProps = Pick<TabularItem, 'icon' | 'label' | 'disabled'> & {
  active?: boolean;
  onClick?(): void;
};

Tabular.Item = React.forwardRef(function TabularItem(props: TabularItemProps, ref: React.Ref<HTMLDivElement>) {
  const {
    icon, label, onClick, active = false, disabled = false,
  } = props;

  const itemClassName = clsx(s.item, {
    [s.active]: active,
    [s.disabled]: disabled,
    [s.iconOnly]: !label,
  });

  const content = (
    <>
      {!!icon && (
        <div className={s.icon}>{icon}</div>
      )}

      {!!label && (
        <div className={s.label}>{label}</div>
      )}
    </>
  );

  return (
    <Interactive ref={ref} onClick={onClick} className={itemClassName}>
      {content}

      <div className={s.content}>{content}</div>

      <div className={s.decorator} />
    </Interactive>
  );
});
