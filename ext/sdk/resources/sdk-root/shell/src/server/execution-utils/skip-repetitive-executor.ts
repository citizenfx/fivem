import { Deferred } from "server/deferred";
import { BoundExecutor } from "./interfaces";

enum SkipRepetitiveExecutorState {
  idle,
  busy,
}

export class SkipRepetitiveExecutor<RetType> implements BoundExecutor<Promise<RetType>> {
  protected state = SkipRepetitiveExecutorState.idle;

  protected pendingExec: Deferred<RetType> | void;

  constructor(
    protected fn: () => Promise<RetType> | RetType,
  ) {
  }

  execute(): Promise<RetType> {
    if (this.state === SkipRepetitiveExecutorState.busy) {
      this.pendingExec ??= new Deferred<RetType>();

      return this.pendingExec.promise;
    }

    this.pendingExec = new Deferred<RetType>();

    this.runWorkCycle();

    return this.pendingExec.promise;
  }

  private async runWorkCycle() {
    while (this.pendingExec) {
      this.state = SkipRepetitiveExecutorState.busy;

      const pendingExec = this.pendingExec;
      this.pendingExec = void 0;

      try {
        pendingExec.resolve(await this.fn());
      } catch (e) {
        pendingExec.reject(e);
      }
    }

    this.state = SkipRepetitiveExecutorState.idle;
  }
}
