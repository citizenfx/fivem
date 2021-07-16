import React from 'react';
import classnames from 'classnames'
import { observer } from 'mobx-react-lite';
import { NotificationState } from 'store/NotificationState';
import { useIframeCover, useOutsideClick } from 'utils/hooks';
import { TaskState } from 'store/TaskState';
import { TaskItem } from './TaskItem/TaskItem';
import { NotificationItem } from './NotificationItem/NotificationItem';
import s from './StatusCenter.module.scss';

export interface StatusCenterProps {
  open: boolean,
  onClose(): void,
  className?: string,
  decorator?: React.ReactNode,
}

const OutsideClickSentinel = React.forwardRef(({ onClose }: { onClose(): void }, ref) => {
  useOutsideClick(ref, onClose);

  useIframeCover();

  return null;
});

export const StatusCenter = observer(function StatusCenter(props: StatusCenterProps) {
  const { onClose, className } = props;

  const ref = React.useRef();

  const tasks = TaskState.values;
  const notifications = NotificationState.values;

  const open = props.open || notifications.length > 0;

  if (!open) {
    ref.current = null;

    return null;
  }

  const rootClassName = classnames(s.root, className, {
    [s.open]: open,
  });

  return (
    <>
      <OutsideClickSentinel ref={ref} onClose={onClose} />

      <div ref={ref} className={rootClassName}>
        {(!notifications.length && !tasks.length) && (
          <div className={s.placeholder}>
            No notifications or tasks!
          </div>
        )}

        {notifications.map((item) => (
          <NotificationItem key={item.id} item={item} />
        ))}

        {tasks.map((item) => (
          <TaskItem key={item.id} item={item} />
        ))}
      </div>

      {props.decorator}
    </>
  );
});
