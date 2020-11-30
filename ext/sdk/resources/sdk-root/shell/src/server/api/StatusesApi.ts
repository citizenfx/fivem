import { SystemEvent, systemEvents } from "./systemEvents";
import { ApiClient } from "shared/api.types";
import { statusesApi } from "shared/api.events";

export class StatusesApi {
  private statuses: Record<string, any> = {};

  constructor(private readonly client: ApiClient) {
    client.on(statusesApi.ack, () => client.emit(statusesApi.statuses, this.statuses));

    systemEvents.on(SystemEvent.setStatus, ({ statusName, statusContent }) => {
      this.statuses[statusName] = statusContent;

      this.ackClient(statusName, statusContent);
    });
  }

  get<T>(statusName: string): T | void {
    return this.statuses[statusName];
  }

  set<T>(statusName: string, statusContent: T) {
    this.statuses[statusName] = statusContent;

    this.ackClient(statusName, statusContent);
  }

  private ackClient(statusName: string, statusContent: any) {
    this.client.emit(statusesApi.update, [statusName, statusContent]);
  }
}

export const setStatus = <T extends any>(statusName: string, statusContent: T) => systemEvents.emit(SystemEvent.setStatus, {
  statusName,
  statusContent,
});
