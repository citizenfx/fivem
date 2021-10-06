import { Api } from "fxdk/browser/Api";
import { makeAutoObservable } from "mobx";
import { notificationsApi } from "shared/api.events";
import { INotificationAction, NotificationItem, NotificationType } from "shared/notification.types";
import { fastRandomId } from "utils/random";

export interface INotification {
  readonly id: string,
  readonly type: NotificationType,
  readonly text: string,
  readonly actions?: INotificationAction[],

  timer?: any,
  animationDuration?: string,
}

export type IClientNotificationOptions = Omit<NotificationItem, 'id' | 'type' | 'text'>;

export const NotificationState = new class NotificationState {
  public items: Record<string, INotification> = {};

  constructor() {
    makeAutoObservable(this);

    Api.on(notificationsApi.create, this.create);
  }

  public get length(): number {
    return Object.keys(this.items).length;
  }

  public get values(): INotification[] {
    return Object.values(this.items);
  }

  public get entries(): [string, INotification][] {
    return Object.entries(this.items);
  }

  public info(text: string, options?: IClientNotificationOptions) {
    this.createLocal(NotificationType.info, text, options);
  }

  public warning(text: string, options?: IClientNotificationOptions) {
    this.createLocal(NotificationType.warning, text, options);
  }

  public error(text: string, options?: IClientNotificationOptions) {
    this.createLocal(NotificationType.error, text, options);
  }

  readonly delete = (id: string) => {
    if (this.items[id]) {
      const notification = this.items[id];

      if (notification.timer) {
        clearTimeout(notification.timer);
      }

      delete this.items[id];
      Api.send(notificationsApi.delete, id);
    }
  };

  private createLocal(type: NotificationType, text: string, options: IClientNotificationOptions = {}) {
    this.create({
      id: fastRandomId(),
      type,
      text,
      ...options,
    });
  }

  private create = (notificationItem: NotificationItem) => {
    const notification: INotification = {
      ...notificationItem,
    };

    if (notificationItem.expireAt) {
      const timeout = notificationItem.expireAt - Date.now();

      if (timeout < 0) {
        return;
      }

      notification.animationDuration = `${timeout}ms`;
      notification.timer = setTimeout(() => this.delete(notification.id), timeout);
    }

    this.items[notification.id] = notification;
  };
}();
