import React from 'react';
import classnames from 'classnames';
import { BsExclamationOctagonFill, BsExclamationTriangle, BsInfoSquare, BsX } from 'react-icons/bs';
import { NotificationType } from 'shared/notification.types';
import { Notification, NotificationState } from 'store/NotificationState';
import s from './NotificationItem.module.scss';

const typeIcons = {
  [NotificationType.info]: <BsInfoSquare />,
  [NotificationType.warning]: <BsExclamationTriangle />,
  [NotificationType.error]: <BsExclamationOctagonFill />,
};

export interface NotificationItemProps {
  item: Notification,
}

export const NotificationItem = React.memo(function NotificationItem({ item }: NotificationItemProps) {
  const rootClassName = classnames(s.root, s[NotificationType[item.type]]);

  const handleClose = React.useCallback(() => {
    NotificationState.delete(item.id);
  }, [item]);

  return (
    <div className={rootClassName}>
      {!!item.animationDuration && (
        <div
          className={s.progress}
          style={{ animationDuration: item.animationDuration }}
        />
      )}

      <div className={s.content}>
        <div className={s.icon}>
          {typeIcons[item.type]}
        </div>
        <div className={s.text}>
          {item.text}
        </div>
      </div>

      <div className={s.actions}>
        <div className={classnames(s.close, s.button)} onClick={handleClose}>
          <BsX />
        </div>
      </div>
    </div>
  );
});
