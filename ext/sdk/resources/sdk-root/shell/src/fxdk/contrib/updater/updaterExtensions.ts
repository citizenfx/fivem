import { Registry } from "fxdk/base/registry";
import { IBaseViewRegistryItem, viewRegistryVisibilityFilter } from "fxdk/base/viewRegistry";

export interface IUpdaterViewParticipant extends IBaseViewRegistryItem {
  shouldShowUpdater?(): boolean;

  handleUpdaterClose?(): void;
}

export const UpdaterViewParticipants = new class UpdaterViewParticipants {
  private registry = new Registry<IUpdaterViewParticipant>('updater-view-participants', true);

  register(participant: IUpdaterViewParticipant) {
    this.registry.register(participant);
  }

  shouldShowUpdater() {
    return this.getAll().some((participant) => {
      return participant.shouldShowUpdater?.() || false;
    });
  }

  handleClose() {
    this.getAll().forEach((participant) => {
      participant.handleUpdaterClose?.();
    });
  }

  getAll() {
    return this.registry.getAll();
  }

  getAllVisible(): IUpdaterViewParticipant[] {
    return this.getAll().filter(viewRegistryVisibilityFilter);
  }
}();
