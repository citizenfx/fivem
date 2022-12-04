import { clsx } from 'cfx/utils/clsx';
import React from 'react';
import s from './Page.module.scss';

export interface PageProps {
  children?: React.ReactNode,
  className?: string,

  showLoader?: boolean,
}

export function Page(props: PageProps) {
  const {
    children,
    className,
    showLoader = false,
  } = props;

  const rootClassName = clsx(s.root, className);

  return (
    <div className={rootClassName}>
      {children}

      {showLoader && (
        <div className={s.loader} />
      )}
    </div>
  );
}
