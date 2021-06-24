import { Indicator } from 'components/Indicator/Indicator';
import { StatusCenter } from 'components/StatusCenter/StatusCenter';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsListTask } from 'react-icons/bs';
import { VscTriangleUp } from 'react-icons/vsc';
import { TaskState } from 'store/TaskState';
import { useOpenFlag } from 'utils/hooks';
import s from './StatusBar.module.scss';

export const StatusBar = observer(function StatusBar() {
  const tasks = TaskState.values;
  const [detailsOpen, _openDetails, closeDetails, toggleDetailsOpen] = useOpenFlag(false);

  const hasTasks = tasks.length > 0;

  return (
    <>
      <div
        className={s.root}
        title="Notifications & background tasks"
        onClick={toggleDetailsOpen}
      >
        {hasTasks ? <Indicator /> : <BsListTask />}
      </div>

      <StatusCenter
        open={detailsOpen}
        onClose={closeDetails}
        className={s['status-center']}
        decorator={
          <div className={s['status-center-decorator']}>
            <VscTriangleUp />
          </div>
        }
      />
    </>
  );
});
