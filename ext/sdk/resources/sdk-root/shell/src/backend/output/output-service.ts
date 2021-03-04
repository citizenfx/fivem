import { inject, injectable } from "inversify";
import { ApiClient } from "backend/api/api-client";
import { ApiContribution } from "backend/api/api-contribution";
import { DisposableContainer, DisposableObject } from "backend/disposable-container";
import { outputApi } from "shared/api.events";
import { OutputChannelProvider } from "./output-types";

@injectable()
export class OutputService implements ApiContribution {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  private readonly channels: Record<string, OutputChannel> = {};

  getId(): string {
    return 'OutputService';
  }

  createOutputChannelFromProvider(provider: OutputChannelProvider): OutputChannel {
    const channelId = provider.getOutputChannelId();
    const disposableContainer = new DisposableContainer();

    const channel = new OutputChannel(
      this.apiClient,
      channelId,
      disposableContainer,
    );

    disposableContainer.add(
      provider.onOutputData((data) => channel.write(data)),
      () => delete this.channels[channelId],
    );

    this.channels[channelId] = channel;

    this.apiClient.emit(outputApi.outputLabel, {
      channelId,
      label: provider.getOutputChannelLabel(),
    });

    return channel;
  }
}

export class OutputChannel implements DisposableObject {
  private disposed = false;

  constructor(
    protected readonly apiClient: ApiClient,
    protected readonly channelId: string,
    protected readonly disposableContainer: DisposableContainer,
  ) { }

  write(data: string | Buffer) {
    if (this.disposed) {
      return;
    }

    if (typeof data === 'string') {
      return this.apiClient.emit(outputApi.output, {
        channelId: this.channelId,
        data,
      });
    }

    return this.apiClient.emit(outputApi.output, {
      channelId: this.channelId,
      data: data.toString(),
    });
  }

  async dispose() {
    this.disposed = true;

    this.apiClient.emit(outputApi.flush, {
      channelId: this.channelId,
    });

    await this.disposableContainer.dispose();
  }
}
