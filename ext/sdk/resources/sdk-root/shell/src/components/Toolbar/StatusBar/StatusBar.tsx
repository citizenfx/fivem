import { Indicator } from 'components/Indicator/Indicator';
import { StatusCenter } from 'components/StatusCenter/StatusCenter';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsListTask } from 'react-icons/bs';
import { VscTriangleDown } from 'react-icons/vsc';
import { TaskData } from 'shared/task.types';
import { TaskState } from 'store/TaskState';
import { useOpenFlag } from 'utils/hooks';
import s from './StatusBar.module.scss';

function getBarTitle(tasks: TaskData[]): string {
  let barTitle = '';

  if (tasks.length === 1) {
    barTitle = tasks[0].title;

    if (tasks[0].text) {
      barTitle += `: ${tasks[0].text}`;
    }
  } else if (tasks.length > 1) {
    barTitle = tasks[0].title + ` and ${tasks.length - 1} more...`;
  }

  return barTitle;
}

export const StatusBar = observer(function StatusBar() {
  const tasks = TaskState.values;
  const [detailsOpen, _openDetails, closeDetails, toggleDetailsOpen] = useOpenFlag(false);

  const hasTasks = tasks.length > 0;
  const barTitle = getBarTitle(tasks);

  return (
    <>
      <div
        className={s.root}
        title="Notifications & background tasks"
        onClick={toggleDetailsOpen}
      >
        <div className={s.icon}>
          {hasTasks ? <Indicator /> : <BsListTask />}
        </div>

        <div className={s.title}>
          {barTitle}
        </div>
      </div>

      <StatusCenter
        open={detailsOpen}
        onClose={closeDetails}
        className={s['status-center']}
        decorator={
          <div className={s['status-center-decorator']}>
            <VscTriangleDown />
          </div>
        }
      />
    </>
  );
});
