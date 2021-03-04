import { Deferred } from "backend/deferred";

export enum FsAtomicWriterState {
  idle,
  busy,
}

export class FsAtomicWriter {
  protected state = FsAtomicWriterState.idle;

  protected pendingWrite: Deferred<void> | void;
  protected pendingContent: string | Buffer | void;

  constructor(
    protected readonly path: string,
    protected readonly fsWriteFile: (path: string, content: string | Buffer) => Promise<void>,
  ) {

  }

  async write(content: string | Buffer): Promise<void> {
    if (this.state === FsAtomicWriterState.busy) {
      this.pendingContent = content;
      this.pendingWrite ??= new Deferred<void>();

      return this.pendingWrite.promise;
    }

    const defer = new Deferred<void>();

    this.flushContent(defer, content);

    return defer.promise;
  }

  private async flushContent(defer: Deferred<void>, content: string | Buffer) {
    this.pendingContent = content;
    this.pendingWrite = defer;

    while (typeof this.pendingContent !== 'undefined' && this.pendingWrite) {
      this.state = FsAtomicWriterState.busy;

      const pendingContent = this.pendingContent;
      const pendingWrite = this.pendingWrite;

      this.pendingContent = void 0;
      this.pendingWrite = void 0;

      try {
        await this.fsWriteFile(this.path, pendingContent);
        pendingWrite.resolve();
      } catch (e) {
        pendingWrite.reject(e);
      }
    }

    this.state = FsAtomicWriterState.idle;
  }
}
