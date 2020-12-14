import { produce } from 'immer';
import { FsAtomicWriter } from "./fs-atomic-writer";

export interface FsJsonFileMappingOptions<T extends object> {
  defaults?: Partial<T>,

  beforeApply?(snapshot: T): void;
  onApply?(snapshot: T): void;
  onWrite?(snapshot: T): void;
}

export class FsJsonFileMapping<T extends object> {
  constructor(
    protected snapshot: T,
    protected readonly writer: FsAtomicWriter,
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

    await this.writer.write(JSON.stringify(snapshot, null, 2));

    this.options?.onWrite?.(snapshot);
  }
}
