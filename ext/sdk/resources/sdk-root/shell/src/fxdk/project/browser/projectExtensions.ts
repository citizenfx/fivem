import { IDisposable } from "fxdk/base/disposable";
import { AssocRegistry, IBaseRegistryItem, Registry } from "fxdk/base/registry";
import { IBaseViewRegistryItem, viewRegistryVisibilityFilter } from "fxdk/base/viewRegistry";
import { IConvarCategoryMap } from "../common/project.types";
import { ProjectStateEvents } from "./state/projectStateEvents";

/**
 * Extension participating in project rendering, see settings, for example
 */
export interface IProjectRenderParticipant extends IBaseViewRegistryItem {
}

/**
 * Extension describing project item (asset, file, directory) creator that will be shown where need be
 */
export interface IProjectItemCreatorParticipant extends IBaseRegistryItem {
  /**
   * Enabled by default
   */
  enabled?(): boolean;

  readonly label: string;
  readonly icon: React.ReactNode;
  readonly commandId: string;
}

interface IBaseProjectControlParticipant extends IBaseRegistryItem {
  /**
   * Enabled by default
   */
  enabled?(): boolean;

  /**
   * Since it is always visible it is worth bleeding this id right here so intro can use this id
   */
  readonly introId?: string;
}
/**
 * Extension describing project controls bar item
 */
export interface IProjectControlParticipant extends IBaseProjectControlParticipant {
  readonly label: string;
  readonly icon: React.ReactNode;
  readonly commandId: string;
  readonly commandArgs?: unknown[];
}
export interface IProjectStackedControlsParticipant extends IBaseProjectControlParticipant {
  readonly icon: React.ReactNode;

  readonly controls: ReadonlyArray<IProjectControlParticipant>;
}

export interface IProjectVariablesProvider {
  label: string,
  getConvarCategories(): IConvarCategoryMap,
}

export const ProjectParticipants = new class ProjectParticipants {
  private renderRegistry = new Registry<IProjectRenderParticipant>('project-view-participants', true);
  private itemCreatorRegistry = new Registry<IProjectItemCreatorParticipant>('project-item-creator-participants', true);
  private controlRegistry = new Registry<IProjectControlParticipant | IProjectStackedControlsParticipant>('project-control-participant', true);
  private variableProvidersRegistry = new AssocRegistry<IProjectVariablesProvider>('project-variables-providers', true);

  constructor() {
    ProjectStateEvents.BeforeClose.addListener(() => this.reset());
  }

  registerRender(participant: IProjectRenderParticipant) {
    this.renderRegistry.register(participant);
  }

  registerItemCreator(participant: IProjectItemCreatorParticipant) {
    this.itemCreatorRegistry.register(participant);
  }

  registerControl(participant: IProjectControlParticipant | IProjectStackedControlsParticipant) {
    this.controlRegistry.register(participant);
  }

  registerDynamicVariablesProvider(id: string, provider: IProjectVariablesProvider): IDisposable {
    this.variableProvidersRegistry.register(id, provider);

    return () => this.unregisterDynamicVariablesProvider(id);
  }

  unregisterDynamicVariablesProvider(id: string) {
    this.variableProvidersRegistry.unregister(id);
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

  getAllVariablesProviders() {
    return this.variableProvidersRegistry.getAll();
  }

  reset() {
    this.variableProvidersRegistry.reset();
  }
}();

function enabledFilter(item: { enabled?(): boolean }): boolean {
  if (item.enabled) {
    return item.enabled();
  }

  return true;
}
