import { Api } from "fxdk/browser/Api";
import { makeAutoObservable } from "mobx";
import { outputApi } from "shared/api.events";

export const OutputState = new class OutputState {
  private channelOutput: Record<string, string> = Object.create(null);
  private channelLabels: Record<string, string> = Object.create(null);

  constructor() {
    makeAutoObservable(this);

    Api.on(outputApi.output, this.appendChannelOutput);
    Api.on(outputApi.outputLabel, this.setChannelLabel);
    Api.on(outputApi.flush, this.flushChannel);
  }

  getOutput(channelId: string): string {
    return this.channelOutput[channelId] || '';
  }

  getLabel(channelId: string): string | void {
    return this.channelLabels[channelId];
  }

  private appendChannelOutput = ({ channelId, data }) => {
    this.channelOutput[channelId] = this.getOutput(channelId) + data;
  };

  private setChannelLabel = ({ channelId, label }) => {
    this.channelLabels[channelId] = label;
  };

  private flushChannel = ({ channelId }) => {
    delete this.channelOutput[channelId];
    delete this.channelLabels[channelId];
  };
}();
