import * as React from 'react';
import classnames from 'classnames';
import { NotificationItem, NotificationType } from 'shared/notification.types';
import { useApiMessage, useStore } from 'utils/hooks';
import { notificationsApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { BsExclamationOctagonFill, BsExclamationTriangle, BsInfoSquare, BsX } from 'react-icons/bs';
import { ToolbarState } from 'store/ToolbarState';
import { observer } from 'mobx-react-lite';
import s from './NotificationsManager.module.scss';

const typeIcons = {
  [NotificationType.info]: <BsInfoSquare />,
  [NotificationType.warning]: <BsExclamationTriangle />,
  [NotificationType.error]: <BsExclamationOctagonFill />,
};

export interface NotificationsStoreItem {
  item: NotificationItem,
  timer?: any,
  animationDuration?: string,
}

export const NotificationsManager = observer(function NotificationsManager() {
  const notifications = useStore<NotificationsStoreItem>({});

  React.useEffect(() => sendApiMessage(notificationsApi.ack), []);

  useApiMessage(notificationsApi.create, (item: NotificationItem) => {
    const storeItem: NotificationsStoreItem = {
      item,
    };

    if (item.expireAt) {
      const timeout = item.expireAt - Date.now();

      // if already expired
      if (timeout <= 0) {
        sendApiMessage(notificationsApi.delete, item.id);

        return;
      }

      storeItem.timer = setTimeout(() => {
        notifications.remove(item.id);
        sendApiMessage(notificationsApi.delete, item.id);
      }, timeout);

      storeItem.animationDuration = `${timeout}ms`;
    }

    notifications.set(item.id, storeItem);
  }, [notifications.set, notifications.remove, notifications.get]);

  const notificationsNodes = Object.entries(notifications.store).map(([id, storeItem]) => {
    const itemClassName = classnames(s.item, s[NotificationType[storeItem.item.type]]);

    const handleClose = () => {
      if (storeItem.timer) {
        clearTimeout(storeItem.timer);
      }

      notifications.remove(id);
      sendApiMessage(notificationsApi.delete, id);
    };

    return (
      <div key={id} className={itemClassName}>
        {!!storeItem.animationDuration && (
          <div
            className={s.progress}
            style={{ animationDuration: storeItem.animationDuration }}
          />
        )}

        <div className={s.content}>
          <div className={s.icon}>
            {typeIcons[storeItem.item.type]}
          </div>
          <div className={s.text}>
            {storeItem.item.text}
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

  const rootStyles: React.CSSProperties = {
    '--toolbar-width': `${ToolbarState.width}px`,
  } as any;

  return (
    <div
      style={rootStyles}
      className={s.root}
    >
      {notificationsNodes}
    </div>
  );
});
