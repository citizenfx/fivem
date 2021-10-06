import React from 'react';
import classnames from 'classnames';
import s from './item.module.scss';

export interface ItemStateProps {
  enabled: boolean,
  running?: boolean,
}

export function ItemState(props: ItemStateProps) {
  const { enabled, running = false } = props;

  const rootClassName = classnames(s.state, {
    [s.enabled]: enabled,
    [s.running]: running,
  });

  return (
    <div
      className={rootClassName}
    />
  );
}
