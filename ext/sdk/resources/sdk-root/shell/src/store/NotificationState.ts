import { makeAutoObservable } from "mobx";
import { notificationsApi } from "shared/api.events";
import { NotificationItem, NotificationType } from "shared/notification.types";
import { onApiMessage, sendApiMessage } from "utils/api";
import { fastRandomId } from "utils/random";

export class Notification {
  constructor(
    public id: string,
    public type: NotificationType,
    public text: string,
    public timer?: any,
    public animationDuration?: string,
  ) {}
}

export const NotificationState = new class NotificationState {
  public items: Record<string, Notification> = {};

  constructor() {
    makeAutoObservable(this);

    onApiMessage(notificationsApi.create, this.create);
  }

  public get length(): number {
    return Object.keys(this.items).length;
  }

  public get values(): Notification[] {
    return Object.values(this.items);
  }

  public get entries(): [string, Notification][] {
    return Object.entries(this.items);
  }

  public ack() {
    sendApiMessage(notificationsApi.ack);
  }

  public error(text: string, expireAt?: number) {
    this.create({
      id: fastRandomId(),
      type: NotificationType.error,
      text,
      expireAt,
    });
  }

  readonly delete = (id: string) => {
    if (this.items[id]) {
      const notification = this.items[id];

      if (notification.timer) {
        clearTimeout(notification.timer);
      }

      delete this.items[id];
      sendApiMessage(notificationsApi.delete, id);
    }
  };

  private create = ({ id, type, text, expireAt }: NotificationItem) => {
    const notification = new Notification(id, type, text);

    if (expireAt) {
      const timeout = expireAt - Date.now();

      if (timeout < 0) {
        return;
      }

      notification.animationDuration = `${timeout}ms`;
      notification.timer = setTimeout(() => this.delete(id), timeout);
    }

    this.items[id] = notification;
  };
}();
