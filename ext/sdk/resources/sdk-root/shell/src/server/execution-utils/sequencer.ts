import { Deferred } from "server/deferred";

enum SequencerState {
  idle,
  busy,
}

enum SequencerPriority {
  parallel,
  blocking,
}

export class Sequencer {
  protected state = SequencerState.idle;

  protected bucket: Record<number, [Function, Deferred<any>][]> = {
    [SequencerPriority.parallel]: [],
    [SequencerPriority.blocking]: [],
  };

  protected parallelExecutionFuse = 0;
  protected blockingExecutionFuse = false;

  getStateString(): string {
    return SequencerState[this.state];
  }

  async executeParallel<T>(fn: () => Promise<T> | T): Promise<T> {
    // Since we're already executing in parallel, we can safely execute nested right now
    if (this.parallelExecutionFuse) {
      return fn();
    }

    return this.execute(SequencerPriority.parallel, fn);
  }

  async executeBlocking<T>(fn: () => Promise<T> | T): Promise<T> {
    if (this.blockingExecutionFuse) {
      throw new Error(`Sequencer:executeBlocking() was called inside of another Sequencer:executeBlocking(), this is a deadlock, tell devs, please`);
    }

    return this.execute(SequencerPriority.blocking, fn);
  }

  protected execute<T extends any[], U>(priority: SequencerPriority, fn: (...args: T) => Promise<U> | U): Promise<U> {
    const defer = new Deferred<U>();

    this.bucket[priority].push([fn, defer]);

    if (this.state === SequencerState.idle) {
      this.runWorkLoop();
    }

    return defer.promise;
  }

  private async runWorkLoop() {
    this.state = SequencerState.busy;

    while (this.bucket[SequencerPriority.parallel].length || this.bucket[SequencerPriority.blocking].length) {
      const parallels = this.bucket[SequencerPriority.parallel];
      this.bucket[SequencerPriority.parallel] = [];

      await Promise.all(parallels.map(async ([fn, defer]) => {
        try {
          this.parallelExecutionFuse++;
          defer.resolve(await fn());
          this.parallelExecutionFuse--;
        } catch(e) {
          defer.reject(e);
        }
      }));

      const blockers = this.bucket[SequencerPriority.blocking];
      this.bucket[SequencerPriority.blocking] = [];

      for (const [fn, defer] of blockers) {
        try {
          this.blockingExecutionFuse = true;
          defer.resolve(await fn());
          this.blockingExecutionFuse = false;
        } catch(e) {
          defer.reject(e);
        }
      }
    }

    this.state = SequencerState.idle;
  }
}
