import { IBaseRegistryItem, Registry } from "fxdk/base/registry";
import { IBaseViewRegistryItem, viewRegistryVisibilityFilter } from "fxdk/base/viewRegistry";

export interface IProjectRenderParticipant extends IBaseViewRegistryItem {
}

export interface IProjectItemCreatorParticipant extends IBaseRegistryItem {
  enabled?(): boolean;

  readonly label: string;
  readonly icon: React.ReactNode;
  readonly commandId: string;
}

export interface IProjectControlParticipant extends IBaseRegistryItem {
  enabled?(): boolean;

  readonly label: string;
  readonly icon: React.ReactNode;
  readonly commandId: string;

  readonly introId?: string;
}

export const ProjectParticipants = new class ProjectParticipants {
  private renderRegistry = new Registry<IProjectRenderParticipant>('project-view-participants', true);
  private itemCreatorRegistry = new Registry<IProjectItemCreatorParticipant>('project-item-creator-participants', true);
  private controlRegistry = new Registry<IProjectControlParticipant>('project-control-participant', true);

  registerRender(participant: IProjectRenderParticipant) {
    this.renderRegistry.register(participant);
  }

  registerItemCreator(participant: IProjectItemCreatorParticipant) {
    this.itemCreatorRegistry.register(participant);
  }

  registerControl(participant: IProjectControlParticipant) {
    this.controlRegistry.register(participant);
  }

  getAllVisibleRenders() {
    return this.renderRegistry.getAll().filter(viewRegistryVisibilityFilter);
  }

  getAllEnabledItemCreators() {
    return this.itemCreatorRegistry.getAll().filter(enabledFilter);
  }

  getAllEnabledControls() {
    return this.controlRegistry.getAll().filter(enabledFilter);
  }
}();

function enabledFilter(item: { enabled?(): boolean }): boolean {
  if(item.enabled) {
    return item.enabled();
  }

  return true;
}
