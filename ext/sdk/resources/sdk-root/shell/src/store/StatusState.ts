import { ResourceStatus } from "assets/resource/resource-types";
import { makeAutoObservable } from "mobx";
import { statusesApi } from "shared/api.events";
import { featuresStatuses } from "shared/api.statuses";
import { Feature } from "shared/api.types";
import { onApiMessage, sendApiMessage } from "utils/api";

const defaultFeaturesStatusContent = Object.create(null);

const defaultResourceStatusContent: ResourceStatus = {
  watchCommands: {},
};

export const StatusState = new class StatusState {
  private statuses: Record<string, any> = Object.create(null);

  constructor() {
    makeAutoObservable(this);

    onApiMessage(statusesApi.statuses, this.setStatuses);
    onApiMessage(statusesApi.update, this.updateStatus);
  }

  public ack() {
    sendApiMessage(statusesApi.ack);
  }

  get<T>(name: string, defaultValue: T): T {
    const content = this.statuses[name];

    if (content === null || content === undefined) {
      return defaultValue;
    }

    return content;
  }

  getResourceStatus(resourcePath: string): ResourceStatus {
    return this.get(`resource-${resourcePath}`, defaultResourceStatusContent);
  }

  getFeature(feature: Feature): boolean | void {
    return this.get(featuresStatuses.state, defaultFeaturesStatusContent)[feature];
  }

  private setStatuses = (statuses) => {
    this.statuses = statuses;
  };

  private updateStatus = ([name, content]) => {
    this.statuses[name] = content;
  };
}();
