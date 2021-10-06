import React from 'react';
import classnames from 'classnames';
import { BsExclamationOctagonFill, BsExclamationTriangle, BsInfoSquare, BsX } from 'react-icons/bs';
import { NotificationType } from 'shared/notification.types';
import { INotification, NotificationState } from 'store/NotificationState';
import s from './NotificationItem.module.scss';
import { ShellCommands } from 'shell-api/commands';

const typeIcons = {
  [NotificationType.info]: <BsInfoSquare />,
  [NotificationType.warning]: <BsExclamationTriangle />,
  [NotificationType.error]: <BsExclamationOctagonFill />,
};

export interface NotificationItemProps {
  item: INotification,
}

export const NotificationItem = React.memo(function NotificationItem({ item }: NotificationItemProps) {
  const rootClassName = classnames(s.root, s[NotificationType[item.type]]);

  const handleClose = React.useCallback(() => {
    NotificationState.delete(item.id);
  }, [item]);

  const handleCommand = React.useCallback((commandId: string) => {
    ShellCommands.invoke(commandId);
    handleClose();
  }, [handleClose]);

  const commandNodes = (item.actions || []).map((action) => (
    <div key={action.id} className={s.command} onClick={() => handleCommand(action.commandId)}>
      {action.label}
    </div>
  ));

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
          <div dangerouslySetInnerHTML={{ __html: item.text.replace(/\n/g, '<br>') }} />

          {!!commandNodes.length && (
            <div className={s.commands}>
              {commandNodes}
            </div>
          )}
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
