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
}
