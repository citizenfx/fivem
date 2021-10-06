import { ResourceStatus } from "assets/resource/resource-types";
import { Api } from "fxdk/browser/Api";
import { makeAutoObservable } from "mobx";
import { statusesApi } from "shared/api.events";
import { featuresStatuses } from "shared/api.statuses";
import { Feature } from "shared/api.types";

const defaultFeaturesStatusContent = Object.create(null);

const defaultResourceStatusContent: ResourceStatus = {
  watchCommands: {},
};

export const StatusState = new class StatusState {
  private statuses: Record<string, any> = Object.create(null);

  constructor() {
    makeAutoObservable(this);

    Api.on(statusesApi.statuses, this.setStatuses);
    Api.on(statusesApi.update, this.updateStatus);
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
