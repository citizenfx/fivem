import { produce } from 'immer';
export interface FsJsonFileMappingOptions<T extends object> {
  defaults?: Partial<T>,

  disablePettyPrint?: boolean,

  beforeApply?(snapshot: T): void;
  onApply?(snapshot: T): void;
  onWrite?(snapshot: T): void;
}

export class FsJsonFileMapping<T extends object> {
  constructor(
    protected snapshot: T,
    protected readonly writer: { write(content: string | Buffer): Promise<void> },
    protected readonly reader: () => T,
    protected readonly options?: FsJsonFileMappingOptions<T>,
  ) { }

  get(): T {
    return this.snapshot;
  }

  async apply(fn: (data: T) => void) {
    const snapshot = produce(this.snapshot, (draft: T) => {
      this.options?.beforeApply?.(draft);
      fn(draft);
    });

    this.snapshot = snapshot;

    this.options?.onApply?.(snapshot);

    if (this.options?.disablePettyPrint) {
      await this.writer.write(JSON.stringify(snapshot));
    } else {
      await this.writer.write(JSON.stringify(snapshot, null, 2));
    }

    this.options?.onWrite?.(snapshot);
  }
}
