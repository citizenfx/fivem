import React from 'react';
import s from './Clickable.module.scss';

export interface ClickableProps {
  onClick: (event: React.MouseEvent<HTMLDivElement>) => void,
  tabIndex?: number,
  children: React.ReactNode,
}

export const Clickable = React.forwardRef(function Clickable(props: ClickableProps, ref: React.Ref<HTMLDivElement>) {
  const {
    onClick,
    tabIndex,
    children,
  } = props;

  return (
    <div
      ref={ref}
      onClick={onClick}
      tabIndex={tabIndex}
      className={s.root}
    >
      {children}
    </div>
  );
});
