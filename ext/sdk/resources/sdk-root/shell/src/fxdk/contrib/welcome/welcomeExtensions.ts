import { Registry } from "fxdk/base/registry";
import { IBaseViewRegistryItem, viewRegistryVisibilityFilter } from "fxdk/base/viewRegistry";

export interface IWelcomeViewParticipant extends IBaseViewRegistryItem {
}

export const WelcomeParticipants = new class WelcomeParticipants {
  private viewRegistry = new Registry<IWelcomeViewParticipant>('welcome-view-participants', true);

  regsiterView(participant: IWelcomeViewParticipant) {
    this.viewRegistry.register(participant);
  }

  getAllVisibleViews() {
    return this.viewRegistry.getAll().filter(viewRegistryVisibilityFilter);
  }
}();
