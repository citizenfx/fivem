import { HotkeyBinding, HotkeyScope, Keystroke } from "utils/HotkeyController";

export type RegisterCommandBindingListener = (binding: HotkeyBinding) => void;

export interface CommandBinding {
  command: string,

  scope?: HotkeyScope,
  extra?: {
    [key: string]: unknown,
  },

  execute(): void,
}

const commandBindings: Record<string, CommandBinding> = {};
const commandKeystrokes: Record<string, Keystroke> = {};
const defaultCommandKeystrokes: Record<string, Keystroke> = {};

const registerListeners: Set<RegisterCommandBindingListener> = new Set();

export function onRegisterCommandBinding(cb: RegisterCommandBindingListener) {
  registerListeners.add(cb);

  return () => registerListeners.delete(cb);
}

export function registerInterruptingCommandBinding(binding: CommandBinding, keystroke: Keystroke) {
  registerCommandBinding({
    ...binding,
    extra: {
      interrupting: true,
    },
  }, keystroke);
}

export function registerCommandBinding(binding: CommandBinding, keystroke: Keystroke) {
  commandBindings[binding.command] = binding;
  defaultCommandKeystrokes[binding.command] = keystroke;

  if (!commandKeystrokes[binding.command]) {
    commandKeystrokes[binding.command] = keystroke;
  }

  registerListeners.forEach((listener) => listener(getCommandHotkeyBinding(binding.command)));
}

export function updateCommandKeystroke(command: string, keystroke: Keystroke) {
  commandKeystrokes[command] = keystroke;

  if (commandBindings[command]) {
    registerListeners.forEach((listener) => listener(getCommandHotkeyBinding(command)));
  }
}

export function executeCommand(command: string) {
  const executor = commandBindings[command]?.execute;
  if (!executor) {
    return;
  }

  executor();
}

export function getCommandBindingKeystroke(command: string): Keystroke | void {
  return commandKeystrokes[command];
}

export function getDefaultCommandBindingKeystroke(command: string): Keystroke | void {
  return defaultCommandKeystrokes[command];
}

export function getAllCommandHotkeyBindings(): HotkeyBinding[] {
  return Object.keys(commandBindings).map(getCommandHotkeyBinding);
}

function getCommandHotkeyBinding(command: string): HotkeyBinding {
  const binding = commandBindings[command];
  const keystroke = commandKeystrokes[command];

  return {
    ...binding.extra,

    command: binding.command,
    scope: binding.scope,

    ...keystroke,
  };
}
