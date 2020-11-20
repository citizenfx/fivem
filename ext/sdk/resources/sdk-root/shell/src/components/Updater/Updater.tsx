import * as React from 'react';
import { useStatus } from 'contexts/StatusContext';
import s from './Updater.module.scss';

const defaultStatus = {
  completed: 0,
  currentFileName: '',
};

export const Update = React.memo(function Update() {
  const status = useStatus('updater', defaultStatus);

  return (
    <div className={s.root}>
      <h1 className={s.title}>
        Preparing FxDK
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
