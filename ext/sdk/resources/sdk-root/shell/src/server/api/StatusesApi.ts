import { SystemEvent, systemEvents } from "./api.events";
import { ApiClient } from "shared/api.types";
import { statusesApi } from "shared/events";

export class StatusesApi {
  private statuses: Record<string, any> = {};

  constructor(client: ApiClient) {
    client.on(statusesApi.ack, () => client.emit(statusesApi.statuses, this.statuses));

    systemEvents.on(SystemEvent.setStatus, ({ statusName, statusContent }) => {
      this.statuses[statusName] = statusContent;

      client.emit(statusesApi.update, [statusName, statusContent]);
    });
  }
}

export const setStatus = <T extends any>(statusName: string, statusContent: T) => systemEvents.emit(SystemEvent.setStatus, {
  statusName,
  statusContent,
});
