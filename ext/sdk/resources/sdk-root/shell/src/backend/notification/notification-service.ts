import { inject, injectable } from "inversify";
import { ApiContribution } from "backend/api/api.extensions";
import { AppContribution } from "backend/app/app.extensions";
import { notificationsApi } from "shared/api.events";
import { NotificationItem, NotificationType } from "shared/notification.types";
import { fastRandomId } from "utils/random";
import { ApiClient } from "backend/api/api-client";
import { handlesClientEvent } from "backend/api/api-decorators";

export interface NotificationCreateRequest {
  type: NotificationType,
  text: string,
  timeout?: number,
}

@injectable()
export class NotificationService implements AppContribution, ApiContribution {
  getId() {
    return 'NotificationService';
  }

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  protected readonly notifications: Record<string, NotificationItem> = {};

  boot() {
    this.apiClient.onClientConnected.addListener(() => this.ack());

    this.apiClient.onEventListenerError.addListener((error: Error) => {
      this.error(`Unexpected error occured: ${error.toString()}`);
    });
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

    this.apiClient.emit(notificationsApi.create, notification);
  }

  @handlesClientEvent(notificationsApi.delete)
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
      this.apiClient.emit(notificationsApi.create, notification);
    });
  }
}
