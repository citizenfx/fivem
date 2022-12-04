import React from 'react';
import s from './Prose.module.scss';

export interface ProseProps {
  children?: React.ReactNode,
}

export function Prose(props: ProseProps) {
  return (
    <div className={s.root}>
      {props.children}
    </div>
  );
}
