import * as React from 'react';
import { useStatus } from 'contexts/StatusContext';
import { updaterStatuses } from 'shared/api.statuses';
import s from './Updater.module.scss';

const defaultStatus = {
  completed: 0,
  currentFileName: '',
};

export const Update = React.memo(function Update() {
  const status = useStatus(updaterStatuses.state, defaultStatus);

  return (
    <div className={s.root}>
      <h1 className={s.title}>
        <ul>
          <li>Preparing FxDK</li>
          <li>Unwrapping singularity</li>
          <li>Merging black holes</li>
          <li>Asserting dominance</li>
          <li>Fixing bugs</li>
          <li>Almost there</li>
          <li>Why are we here</li>
          <li>What is the point?</li>
          <li>Okay, this takes way too much time 😒</li>
        </ul>
      </h1>

      <div className={s['current-file-name']}>
        {status.currentFileName}
      </div>

      <div className={s.progress}>
        <div className={s.bar} style={{ width: `${status.completed * 100}%` }}/>
      </div>
    </div>
  );
});
