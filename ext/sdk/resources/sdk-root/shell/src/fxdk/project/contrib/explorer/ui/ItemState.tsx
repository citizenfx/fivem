import React from 'react';
import classnames from 'classnames';
import { IProjectExplorer } from '../projectExplorerItem';
import s from '../item.module.scss';

export function ItemState(props: IProjectExplorer.ItemState) {
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
