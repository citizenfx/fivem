export type OutputListener = (data: string | Buffer) => void;
export type OutputListenerDisposer = () => void;

export interface OutputChannelProvider {
  getOutputChannelId(): string;
  onOutputData(listener: OutputListener): OutputListenerDisposer;
}
