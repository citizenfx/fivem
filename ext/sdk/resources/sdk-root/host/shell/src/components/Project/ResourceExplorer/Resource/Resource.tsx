import React from 'react';
import classnames from 'classnames';
import { ResourceDeleter } from './ResourceDeleter/ResourceDeleter';
import { ResourceRenamer } from './ResourceRenamer/ResourceRenamer';
import { useOpenFlag } from '../../../../utils/hooks';
import { ContextMenu, ContextMenuItem } from '../../../controls/ContextMenu/ContextMenu';
import { Button } from '../../../controls/Button/Button';
import { sendApiMessage } from '../../../../utils/api';
import { ProjectItemProps, renderChildren } from '../ResourceExplorer.item';
import { ResourceContext } from './Resource.context';
import { deleteIcon, disabledResourceIcon, enabledResourceIcon, refreshIcon, renameIcon, resourceIcon } from '../../../../constants/icons';
import { ServerContext } from '../../../../contexts/ServerContext';
import s from './Resource.module.scss';
import { projectApi, serverApi } from '../../../../sdkApi/events';
import { FilesystemEntry } from '../../../../sdkApi/api.types';
import { useExpandablePath } from '../ResourceExplorer.hooks';


const resourceChildrenFilter = (entry: FilesystemEntry) => {
  if (entry.name === 'fxasset.json') {
    return false;
  }

  return true;
};

export interface ResourceProps {
  name: string,
  path: string,
};

export const Resource = React.memo((props: ProjectItemProps) => {
  const { entry, project, projectResources } = props;
  const projectResource = projectResources[entry.path];

  const { resourcesState } = React.useContext(ServerContext);
  const { expanded, toggleExpanded } = useExpandablePath(entry.path);

  const isEnabled = !!projectResource?.enabled;

  const { forbidDeletion, forbidRenaming } = React.useContext(ResourceContext);

  const [deleterOpen, openDeleter, closeDeleter] = useOpenFlag(false);
  const [renamerOpen, openRenamer, closeRenamer] = useOpenFlag(false);

  const handleToggle = React.useCallback(() => {
    sendApiMessage(projectApi.setResourceEnabled, {
      projectPath: project.path,
      resourceName: entry.name,
      enabled: !isEnabled,
    });
  }, [entry, project, isEnabled]);

  const handleRestart = React.useCallback(() => {
    sendApiMessage(serverApi.restartResource, entry.name);
  }, [entry]);

  const contextMenuItems: ContextMenuItem[] = [
    {
      id: 'toggle',
      icon: isEnabled
        ? disabledResourceIcon
        : enabledResourceIcon,
      text: isEnabled
        ? 'Disable resource'
        : 'Enable resource',
      onClick: handleToggle,
    },
    {
      id: 'delete',
      icon: deleteIcon,
      text: 'Delete resource',
      disabled: forbidDeletion,
      onClick: openDeleter,
    },
    {
      id: 'rename',
      icon: renameIcon,
      text: 'Rename resource',
      disabled: forbidRenaming,
      onClick: openRenamer,
    },
  ];

  const iconTitle = projectResource?.enabled
    ? 'Enabled'
    : 'Disabled';

  const iconsClassName = classnames(s.icon, {
    [s.active]: resourcesState[entry.name],
  });

  const icon = projectResource?.enabled
    ? enabledResourceIcon
    : resourceIcon;

  const children = renderChildren(entry, props, resourceChildrenFilter);

  return (
    <div className={s.root}>
      <ContextMenu
        className={s.resource}
        activeClassName={s.active}
        items={contextMenuItems}
        onClick={toggleExpanded}
      >
        <div className={s.name}>
          <span className={iconsClassName} title={iconTitle}>
            {icon}
          </span>
          {entry.name}
        </div>

        <div className={s.controls}>
          <Button
            theme="transparent"
            text="restart"
            icon={refreshIcon}
            onClick={handleRestart}
          />
        </div>
      </ContextMenu>

      {expanded && (
        <div className={s.children}>
          {children}
        </div>
      )}

      {deleterOpen && (
        <ResourceDeleter path={entry.path} name={entry.name} onClose={closeDeleter} />
      )}
      {renamerOpen && (
        <ResourceRenamer path={entry.path} name={entry.name} onClose={closeRenamer} />
      )}
    </div>
  );
});
