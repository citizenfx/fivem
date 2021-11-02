import React from 'react';
import { Changelog } from './Changelog';
import s from './ChangelogUpdaterView.module.scss';

export const ChangelogUpdaterView = function ChangelogUpdaterView() {
  return (
    <div className={s.root}>
      <Changelog />
    </div>
  );
};
