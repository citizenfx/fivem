import React from 'react';
import classnames from 'classnames';
import s from './ItemStatusEntry.module.scss';
import { Title } from 'fxdk/ui/controls/Title/Title';

export interface ItemStatusEntryProps {
  icon: React.ReactNode,
  label?: React.ReactNode,
  title?: string,
  className?: string,
  onClick?(): void,
}

export function ItemStatusEntry(props: ItemStatusEntryProps) {
  const { icon, label, title, className, onClick } = props;

  const rootClassName = classnames(s.root, className, {
    [s.clickable]: !!onClick,
  });

  const handleClick = React.useCallback((event) => {
    event.preventDefault();
    event.stopPropagation();

    onClick?.();
  }, [onClick]);

  return (
    <Title title={title} delay={0} fixedOn="top" animated={false}>
      {(ref) => (
        <div ref={ref} className={rootClassName} onClick={handleClick}>
          <div className={s.icon}>
            {icon}
          </div>

          {!!label && (
            <div className={s.label}>
              {label}
            </div>
          )}
        </div>
      )}
    </Title>
  );
}
