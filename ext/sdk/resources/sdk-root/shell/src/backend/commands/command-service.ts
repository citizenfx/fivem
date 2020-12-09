import { injectable, interfaces } from "inversify";

export type CommandExecuter = <T extends any>(data: T) => any;
export type CommandExecuterDisposer = () => void;

export type CommandId = string;

@injectable()
export class CommandService {
  protected readonly executers: Record<CommandId, Set<CommandExecuter>> = {};

  registerExecuter(commandId: CommandId, executer: CommandExecuter): CommandExecuterDisposer {
    if (!this.executers[commandId]) {
      this.executers[commandId] = new Set();
    }

    this.executers[commandId].add(executer);

    return () => this.executers[commandId].delete(executer);
  }

  unregisterExecuter(commandId: CommandId, executer: CommandExecuter) {
    if (!this.executers[commandId]) {
      return;
    }

    this.executers[commandId].delete(executer);
  }

  execute<T extends any>(commandId: CommandId, data: T) {
    this.executers[commandId]?.forEach((executer) => executer(data));
  }
}

export const bindCommandService = (container: interfaces.Container) => {
  container.bind(CommandService).toSelf().inSingletonScope();
};
