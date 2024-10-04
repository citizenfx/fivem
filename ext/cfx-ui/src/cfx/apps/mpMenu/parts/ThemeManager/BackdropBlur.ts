import { makeAutoObservable } from 'mobx';

export const BackdropBlurWorker = new (class BackdropBlurWorker {
  protected worker: Worker;

  private _url: string = '';
  public get url(): string {
    return this._url;
  }
  private set url(url: string) {
    this._url = url;
  }

  constructor() {
    makeAutoObservable(this);

    this.worker = new Worker(new URL('./BackdropBlur.worker.ts', import.meta.url));
    this.worker.addEventListener('message', this.handleMessage);
  }

  public setUrl(url: string) {
    if (this.url) {
      try {
        URL.revokeObjectURL(this.url);
      } catch (e) {
        // noop
      }
      this.url = '';
    }

    if (url) {
      this.worker.postMessage(url);
    }
  }

  private readonly handleMessage = ({
    data,
  }: MessageEvent) => {
    this.url = data || '';
  };
})();
