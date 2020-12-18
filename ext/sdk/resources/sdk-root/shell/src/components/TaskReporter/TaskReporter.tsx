import * as React from 'react';
import classnames from 'classnames';
import { TaskData } from 'shared/task.types';
import { useOpenFlag } from 'utils/hooks';
import { Indicator } from 'components/Indicator/Indicator';
import { TaskContext } from 'contexts/TaskContext';
import s from './TaskReporter.module.scss';

export const TaskReporter = React.memo(function TaskReporter() {
  const { tasks } = React.useContext(TaskContext);
  const [detailsOpen, _openDetails, closeDetails, toggleDetailsOpen] = useOpenFlag(false);

  React.useEffect(() => {
    if (tasks.length === 0) {
      closeDetails();
    }
  }, [tasks, closeDetails]);

  const rootClassName = classnames(s.root, {
    [s.open]: detailsOpen,
    [s['with-bar']]: tasks.length > 0,
  });

  let barTitle = '';
  if (tasks.length === 1) {
    barTitle = tasks[0].title;
  } else if (tasks.length > 1) {
    barTitle = tasks[0].title + ` and ${tasks.length - 1} more...`;
  }

  return (
    <div className={rootClassName} onClick={toggleDetailsOpen}>
      <div className={s.bar}>
        <div className={s.indicator}>
          <Indicator />
        </div>
        <div className={s.title} title={barTitle}>
          <span>{barTitle}</span>
        </div>
      </div>

      <div className={s.details}>
        {tasks.map((task) => (
          <TaskItem key={task.id} task={task} />
        ))}
      </div>
    </div>
  );
});

function TaskItem({ task }: { task: TaskData }) {
  return (
    <div className={s.task}>
      <div className={s.progress} style={{ width: `${task.progress * 100}%` }} />
      <div className={s.content}>
        <div className={s.title}>
          {task.title}
        </div>
        <div className={s.text}>
          {task.text}
        </div>
      </div>
    </div>
  );
}
