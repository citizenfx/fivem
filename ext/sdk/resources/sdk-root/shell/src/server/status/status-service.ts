import { inject, injectable } from "inversify";
import { ApiClient } from "server/api/api-client";
import { statusesApi } from "shared/api.events";
import { handlesClientEvent } from "server/api/api-decorators";
import { ApiContribution } from "server/api/api-contribution";

@injectable()
export class StatusService implements ApiContribution {
  getId() {
    return 'StatusService';
  }

  private statuses: Record<string, any> = {};

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  get<T>(statusName: string): T | void {
    return this.statuses[statusName];
  }

  set<T>(statusName: string, statusContent: T) {
    this.statuses[statusName] = statusContent;

    this.ackClient(statusName, statusContent);
  }

  @handlesClientEvent(statusesApi.ack)
  private ack() {
    this.apiClient.emit(statusesApi.statuses, this.statuses);
  }

  private ackClient(statusName: string, statusContent: any) {
    this.apiClient.emit(statusesApi.update, [statusName, statusContent]);
  }
}
