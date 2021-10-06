import { Registry } from "fxdk/base/registry";
import { IBaseViewRegistryItem, viewRegistryVisibilityFilter } from "fxdk/base/viewRegistry";

export interface IShellViewParticipant extends IBaseViewRegistryItem {
}

export const ShellViewParticipants = new class ShellViewParticipants {
  private registry = new Registry<IShellViewParticipant>('shell-view-participants', true);

  register(participant: IShellViewParticipant) {
    this.registry.register(participant);
  }

  getAll(): IShellViewParticipant[] {
    return this.registry.getAll();
  }

  getAllVisible(): IShellViewParticipant[] {
    return this.getAll().filter(viewRegistryVisibilityFilter);
  }
}();
