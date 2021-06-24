import React from 'react';
import classnames from 'classnames';
import { TaskData } from 'shared/task.types';
import s from './TaskItem.module.scss';

export interface TaskItemProps {
  item: TaskData,
}

export const TaskItem = React.memo(function TaskItem({ item }: TaskItemProps) {
  const progressClassName = classnames(s.progress, {
    [s.indeterminate]: item.progress === 0,
  });

  return (
    <div className={s.root}>
      <div className={s.title}>
        {item.title}
        <div className={s.text}>
          {item.text}
        </div>
      </div>
      <div className={progressClassName}>
        <div className={s.bar} style={{ width: `${item.progress * 100}%` }} />
      </div>
    </div>
  );
});
