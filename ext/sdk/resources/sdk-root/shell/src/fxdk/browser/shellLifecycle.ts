export enum ShellLifecyclePhase {
  Loading,
  Booting,
  Started,
}

export const ShellLifecycle = new class ShellLifecycle {
  private currentPhase = ShellLifecyclePhase.Loading;

  private phaseListeners: Record<number, Function[]> = {};

  onPhase(phase: ShellLifecyclePhase, listener: Function) {
    if (this.currentPhase >= phase) {
      return listener();
    }

    this.phaseListeners[phase] ??= [];
    this.phaseListeners[phase].push(listener);
  }

  advancePhase() {
    const nextPhase = this.currentPhase + 1;

    const msg = `Shell phase ${ShellLifecyclePhase[nextPhase]} took`;
    console.time(msg);

    if (!ShellLifecyclePhase[nextPhase]) {
      throw new Error(`Unable to advance shell lifecycle phase, it is already the latest phase`);
    }

    this.currentPhase = nextPhase;

    const listeners = this.phaseListeners[this.currentPhase];
    if (listeners) {
      for (const listener of listeners) {
        listener();
      }
    }

    console.timeEnd(msg);
  }
}();
