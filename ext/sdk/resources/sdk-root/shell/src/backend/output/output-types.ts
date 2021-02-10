export type OutputListener = (data: string | Buffer) => void;
export type OutputListenerDisposer = () => void;

export interface OutputChannelProvider {
  getOutputChannelId(): string;
  getOutputChannelLabel(): string;

  onOutputData(listener: OutputListener): OutputListenerDisposer;
}
