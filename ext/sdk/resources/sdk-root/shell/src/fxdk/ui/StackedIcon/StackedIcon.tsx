import React from 'react';
import classnames from 'classnames';
import s from './StackedIcon.module.scss';

export interface StackedIconProps {
  base: React.ReactNode,
  complement: React.ReactNode,
  className?: string,
}

export function StackedIcon(props: StackedIconProps) {
  const { base, complement, className } = props;

  return (
    <div className={classnames(s.root, className)}>
      <div className={s.base}>
        {base}
      </div>
      <div className={s.complement}>
        {complement}
      </div>
    </div>
  );
}
