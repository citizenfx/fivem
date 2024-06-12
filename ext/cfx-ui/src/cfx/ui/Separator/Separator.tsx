import React from 'react';

import { clsx } from 'cfx/utils/clsx';

import s from './Separator.module.scss';

export interface SeparatorProps {
  thin?: boolean;
  vertical?: boolean;

  content?: React.ReactNode;
}

export function Separator(props: SeparatorProps) {
  const {
    content, thin = false, vertical = false,
  } = props;

  const rootClassName = clsx(s.root, {
    [s.text]: !!content,
    [s.thin]: thin,
    [s.vertical]: vertical,
  });

  if (content) {
    return (
      <div className={rootClassName}>
        <div className={s.separator} />
        <div className={s.content}>{content}</div>
        <div className={s.separator} />
      </div>
    );
  }

  return (
    <div className={rootClassName}>
      <div className={s.separator} />
    </div>
  );
}
