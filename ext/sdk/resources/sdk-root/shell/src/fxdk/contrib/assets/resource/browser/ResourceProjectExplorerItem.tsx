import React from 'react';
import { SystemResource, SYSTEM_RESOURCES_NAMES } from 'backend/system-resources/system-resources-constants';
import { checkedIcon, deleteIcon, closedResourceIcon, refreshIcon, startIcon, stopIcon, uncheckedIcon, openResourceIcon } from 'fxdk/ui/icons';
import { ProjectCommands } from 'fxdk/project/browser/project.commands';
import { IProjectExplorer } from 'fxdk/project/contrib/explorer/projectExplorerItem';
import { NativeTypes } from 'react-dnd-html5-backend';
import { OpenFlag } from 'store/generic/OpenFlag';
import { ServerState } from 'store/ServerState';
import { StatusState } from 'store/StatusState';
import { ResourceAssetConfig, ResourceStatus } from '../common/resource.types';
import { ResourceCommands } from './resource.commands';
import { BsExclamationCircle, BsFillEyeFill } from 'react-icons/bs';
import { ItemStatusEntry } from 'fxdk/project/contrib/explorer/ui/ItemStatusEntry/ItemStatusEntry';
import { ResourceCommandsOutputModal } from './ResourceCommandsOutputModal/ResourceCommandsOutputModal';
import { ShellCommands } from 'shell-api/commands';
import { resourceNamePattern } from 'constants/patterns';
import { ProjectExplorerItemMenuGroups } from 'fxdk/project/contrib/explorer/explorer.itemMenu';
import { PathState } from 'fxdk/project/browser/state/primitives/PathState';
import { ProjectExplorerFileSystemItem } from 'fxdk/project/contrib/explorer/explorer.fileSystemItem';
import { projectExplorerItemType } from 'fxdk/project/contrib/explorer/explorer.dnd';
import { IFsEntry } from 'fxdk/project/common/project.types';
import { Indicator } from 'fxdk/ui/Indicator/Indicator';
import { Project } from 'fxdk/project/browser/state/project';
import { IResourceRuntimeData } from '../common/resourceRuntimeData';
import { ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import s from './Resource.module.scss';
import { disposableIdleCallback } from 'fxdk/base/async';
import { StackedIcon } from 'fxdk/ui/StackedIcon/StackedIcon';
import { VscRefresh } from 'react-icons/vsc';

const defaultResourceAssetConfig: ResourceAssetConfig = {
  enabled: false,
  restartOnChange: false,
};

export class ResourceProjectExplorerItem extends ProjectExplorerFileSystemItem<IResourceRuntimeData> {
  static readonly IN_RESOURCE_CONTEXT_KEY = Symbol('InResource');

  readonly pathState = new PathState(this.entryPath, true);

  private commandOutputState = new OpenFlag(false);
  private commandOutputCommand = this.toDispose.register(ShellCommands.registerDynamic('showCommandsOutput', this.commandOutputState.open));

  private get watchCommands() {
    return this.getResourceStatus().watchCommands;
  }

  private get isRunning(): boolean {
    return ServerState.isResourceRunning(this.entry.name);
  }

  private get isEnabled(): boolean {
    return this.getConfig().enabled;
  }

  private get isRestartOnChangeEnabled(): boolean {
    return this.getConfig().restartOnChange;
  }

  constructor(entry: IFsEntry, entryPath: string) {
    super({
      entry,
      entryPath,
      rename: {
        validator: resourceNamePattern,
      },
      options: {
        noDefaultDelete: true,
      },
      menuItems: [
        {
          id: 'restart-on-change-disable',
          icon: checkedIcon,
          group: ProjectExplorerItemMenuGroups.ASSET,
          label: 'Disable autorestart',
          disabled: () => this.getIsConflictingWithSystemResource(),
          visible: () => this.isRestartOnChangeEnabled,
          commandId: ResourceCommands.DISABLE_RESTART_ON_CHANGE,
          commandArgs: () => [entryPath],
        },
        {
          id: 'restart-on-change-enable',
          icon: uncheckedIcon,
          group: ProjectExplorerItemMenuGroups.ASSET,
          label: 'Enable autorestart',
          disabled: () => this.getIsConflictingWithSystemResource(),
          visible: () => !this.isRestartOnChangeEnabled,
          commandId: ResourceCommands.ENABLE_RESTART_ON_CHANGE,
          commandArgs: () => [entryPath],
        },
        {
          id: 'delete-resource',
          icon: deleteIcon,
          label: 'Delete',
          group: ProjectExplorerItemMenuGroups.FSOPS,
          order: 4,
          commandId: ProjectCommands.DELETE_ENTRY_CONFIRM_FIRST,
          commandArgs: () => [entryPath, `Delete "${this.entry.name}" resource?`, () => null],
        },
        {
          id: 'resource-stop',
          icon: stopIcon,
          label: 'Stop',
          group: '3_resource_lifecycle',
          order: 1,
          visible: () => ServerState.isUp && this.isRunning,
          commandId: ResourceCommands.STOP_RESOURCE,
          commandArgs: () => [this.entry.name],
        },
        {
          id: 'resource-start',
          icon: startIcon,
          label: 'Start',
          group: '3_resource_lifecycle',
          order: 2,
          disabled: () => !ServerState.isUp,
          visible: () => !this.isRunning,
          commandId: ResourceCommands.START_RESOURCE,
          commandArgs: () => [this.entry.name],
        },
        {
          id: 'resource-restart',
          icon: refreshIcon,
          label: 'Restart',
          group: '3_resource_lifecycle',
          order: 3,
          visible: () => ServerState.isUp && this.isRunning,
          commandId: ResourceCommands.RESTART_RESOURCE,
          commandArgs: () => [this.entry.name],
        },
        {
          id: 'show-commands-output',
          icon: <BsFillEyeFill />,
          label: 'Watch commands output',
          group: ProjectExplorerItemMenuGroups.EXTRAS,
          visible: () => !!Object.keys(this.watchCommands).length,
          commandId: () => this.commandOutputCommand.id,
        },
      ],
      dragAndDrop: {
        drag: {
          getItem: () => ({
            type: projectExplorerItemType.ASSET,
            entryPath: this.entryPath,
          }),
        },

        drop: {
          acceptTypes: [
            projectExplorerItemType.FILE,
            projectExplorerItemType.FOLDER,
            NativeTypes.FILE,
          ],
        },
      },
    });

    this.register(disposableIdleCallback(() => {
      this.register(ProjectParticipants.registerDynamicVariablesProvider(this.entryPath, {
        label: Project.getRelativePath(this.entryPath),
        getConvarCategories: () => {
          return this.runtimeData?.convarCategories || {};
        },
      }));
    }));
  }

  private getIsConflictingWithSystemResource = () => {
    return Project.manifest.systemResources.includes(this.entry.name as SystemResource);
  };

  override acceptParentContext(parentContext: IProjectExplorer.ItemContext): IProjectExplorer.ItemContext {
    return super.acceptParentContext({
      ...parentContext,
      [ResourceProjectExplorerItem.IN_RESOURCE_CONTEXT_KEY]: true,
    });
  }

  getDefaultIcon(expanded: boolean) {
    return expanded
      ? openResourceIcon
      : closedResourceIcon;
  }

  getIcon() {
    if (this.getIsConflictingWithSystemResource()) {
      return {
        className: s.conflicting,
        icon: <BsExclamationCircle />,
      };
    }

    if (!this.runtimeData?.ready) {
      return <Indicator />;
    }

    return {
      className: this.isRunning ? s.running : '',
      icon: this.pathState.isExpanded
        ? openResourceIcon
        : closedResourceIcon,
    };
  }

  getLabel() {
    return this.entry.name;
  }

  getTitle() {
    return this.getIsConflictingWithSystemResource()
      ? `"${SYSTEM_RESOURCES_NAMES[this.entry.name as SystemResource]}" is enabled, this resource will be ignored. Either rename this resource or disable "${SYSTEM_RESOURCES_NAMES[this.entry.name as SystemResource]}" in Project Settings`
      : `${this.entry.name} • ${this.getConfig().enabled ? 'Enabled' : 'Disabled'}${this.isRunning ? ' • Running' : ''}`;
  }

  getState(): IProjectExplorer.ItemState {
    return {
      enabled: this.isEnabled,
      running: this.isRunning,
    };
  }

  getStatus() {
    let devtoolsRunningNode: React.ReactNode = null;

    const watchCommands = Object.values(this.watchCommands);
    if (watchCommands.length) {
      const runningCommandsLength = watchCommands.filter((cmd) => cmd.running).length;

      devtoolsRunningNode = (
        <ItemStatusEntry
          key="devtools"
          icon={<BsFillEyeFill />}
          label={`${runningCommandsLength}/${watchCommands.length}`}
          title="Watch commands: running / declared"
          onClick={this.commandOutputState.open}
        />
      );
    }

    return [
      devtoolsRunningNode,
      <ItemStatusEntry
        key="autorestart"
        icon="A"
        title={
          this.isRestartOnChangeEnabled
            ? 'Autorestart enabled'
            : 'Autorestart disabled'
        }
        onClick={this.toggleRestartOnChange}
        className={
          this.isRestartOnChangeEnabled
            ? 'text-scColor'
            : ''
        }
      />
    ].filter(Boolean);
  }

  renderInWrapper() {
    if (this.commandOutputState.isOpen) {
      return (
        <ResourceCommandsOutputModal
          onClose={this.commandOutputState.close}
          resourceName={this.entry.name}
          resourceStatus={this.getResourceStatus()}
        />
      );
    }
  }

  shouldRenderChildren() {
    return this.pathState.isExpanded;
  }

  readonly handleClick = () => this.pathState.toggle();

  private readonly toggleRestartOnChange = () => {
    if (this.isRestartOnChangeEnabled) {
      ShellCommands.invoke(ResourceCommands.DISABLE_RESTART_ON_CHANGE, this.entryPath);
    } else {
      ShellCommands.invoke(ResourceCommands.ENABLE_RESTART_ON_CHANGE, this.entryPath);
    }
  };

  private getConfig(): ResourceAssetConfig {
    return {
      ...defaultResourceAssetConfig,
      ...Project.getAssetConfig(this.entryPath),
    }
  }

  private getResourceStatus(): ResourceStatus {
    return StatusState.getResourceStatus(this.entryPath);
  }
}
