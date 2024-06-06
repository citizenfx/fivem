import React from 'react';

import s from './Prose.module.scss';

export interface ProseProps {
  children?: React.ReactNode;
}

export function Prose({
  children,
}: ProseProps) {
  return (
    <div className={s.root}>{children}</div>
  );
}
