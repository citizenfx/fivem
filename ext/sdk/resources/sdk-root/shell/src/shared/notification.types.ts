
export interface INotificationAction {
  id: string,
  label: string,
  commandId: string,
}

export enum NotificationType {
  info,
  warning,
  error,
}

export interface NotificationItem {
  id: string,
  type: NotificationType,
  text: string,
  expireAt?: number,
  actions?: INotificationAction[],
}
