import { Deferred } from "backend/deferred";

export interface FsThrottledWriterOptions<ContentType> {
  serialize(content: ContentType): string,

  time: number,
}

export class FsThrottledWriter<ContentType> {
  private pendingDeferred: Deferred<void> | null = null;
  private pendingContent: ContentType | null = null;

  private timer: NodeJS.Timeout;

  constructor(
    private writer: (content: string | Buffer) => Promise<void>,
    private options: FsThrottledWriterOptions<ContentType>,
  ) {

  }

  write(content: ContentType) {
    this.pendingContent = content;

    if (!this.timer) {
      this.timer = setTimeout(this.flushContent, this.options.time);
    }
  }

  async flush() {
    if (this.pendingDeferred) {
      await this.pendingDeferred.promise;
    }

    if (this.timer) {
      clearTimeout(this.timer);

      return this.flushContent();
    }
  }

  private flushContent = async () => {
    this.timer = undefined;

    if (this.pendingDeferred) {
      await this.pendingDeferred.promise;
    }

    this.pendingDeferred = new Deferred();

    await this.writer(this.options.serialize(this.pendingContent));

    this.pendingDeferred.resolve();
    this.pendingDeferred = undefined;
  };
}
