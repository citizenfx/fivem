import { notificationsApi } from "shared/api.events";
import { ApiClient } from "shared/api.types";
import { NotificationItem, NotificationType } from "shared/notification.types";
import { fastRandomId } from "utils/random";

export class NotificationsApi {
  private readonly notifications: Record<string, NotificationItem> = {};

  constructor(private readonly client: ApiClient) {
    this.client.on(notificationsApi.ack, () => this.ack());
    this.client.on(notificationsApi.delete, (id: string) => this.delete(id));
  }

  create(type: NotificationType, text: string, timeout?: number) {
    const id = fastRandomId();

    const notification: NotificationItem = {
      id,
      type,
      text,
    };

    if (timeout) {
      notification.expireAt = Date.now() + timeout;
    }

    this.notifications[id] = notification;

    this.client.emit(notificationsApi.create, notification);
  }

  delete(id: string) {
    delete this.notifications[id];
  }

  info(text: string, timeout?: number) {
    return this.create(NotificationType.info, text, timeout);
  }

  warning(text: string, timeout?: number) {
    return this.create(NotificationType.warning, text, timeout);
  }

  error(text: string, timeout?: number) {
    return this.create(NotificationType.error, text, timeout);
  }

  private ack() {
    Object.values(this.notifications).forEach((notification) => {
      this.client.emit(notificationsApi.create, notification);
    });
  }
}
