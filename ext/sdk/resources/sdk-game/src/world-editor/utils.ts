export function getSmartControlNormal(control: number | number[]): number {
  if (Array.isArray(control)) {
    return GetDisabledControlNormal(0, control[0]) - GetDisabledControlNormal(0, control[1]);
  }

  return GetDisabledControlNormal(0, control);
}

export function useKeyMapping(cmd: string, desc: string, device: string, key: string) {
  const onHandlers = new Set<Function>();
  const offHandlers = new Set<Function>();

  const state = {
      isActive: false,
      on: (cb) => {
          onHandlers.add(cb);

          return () => onHandlers.delete(cb);
      },
      off: (cb) => {
          offHandlers.add(cb);

          return () => offHandlers.delete(cb);
      },
  };

  RegisterCommand('+' + cmd, () => {
      state.isActive = true;

      const handlers = new Set(onHandlers);

      handlers.forEach((handler) => handler());
  }, false);

  RegisterCommand('-' + cmd, () => {
      state.isActive = false;

      const handlers = new Set(offHandlers);

      handlers.forEach((handler) => handler());
  }, false);

  RegisterKeyMapping('+' + cmd, desc, device, key);

  return state;
}

export function drawDebugText(text: string, x: number = .5, y: number = 0) {
  SetTextFont(4);
  SetTextProportional(true);
  SetTextScale(0, 0.5);
  SetTextColour(255, 255, 255, 255);
  SetTextDropShadow();
  SetTextOutline();
  SetTextEntry('STRING');
  AddTextComponentString(text);

  DrawText(x, y);
}

export class Memoizer<T> {
  private s: string;

  constructor(
    initial: T,
    private serializer: ((v: T) => string) = (v: T) => JSON.stringify(v),
  ) {
    this.s = serializer(initial);
  }

  compareAndStore(v: T): boolean {
    const s = this.serializer(v);
    let ret = true;

    if (this.s !== s) {
      ret = false;
    }

    this.s = s;

    return ret;
  }
}
