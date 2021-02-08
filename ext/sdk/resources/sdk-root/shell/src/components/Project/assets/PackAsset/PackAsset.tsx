import React from 'react';
import {
  ProjectExplorerItemContext,
  ProjectExplorerItemContextProvider,
  ProjectExplorerVisibilityFilter,
} from 'components/Project/ProjectExplorer/item.context';
import { ProjectItemProps } from 'components/Project/ProjectExplorer/item';
import { invariant } from 'utils/invariant';
import { useExpandablePath, useItem } from 'components/Project/ProjectExplorer/ProjectExplorer.hooks';
import { assetStatus } from 'shared/api.types';
import { assetIcon, rotatingRefreshIcon } from 'constants/icons';
import { combineVisibilityFilters, visibilityFilters } from 'components/Explorer/Explorer.filters';
import { itemsStyles } from 'components/Project/ProjectExplorer/item.styles';
import { ContextMenu } from 'components/controls/ContextMenu/ContextMenu';


const visibilityFilter = combineVisibilityFilters(
  ProjectExplorerVisibilityFilter,
  visibilityFilters.hideFiles,
);

const contextOptions: Partial<ProjectExplorerItemContext> = {
  disableAssetCreate: true,
  disableAssetDelete: true,
  disableAssetRename: true,
  disableDirectoryCreate: true,
  disableDirectoryDelete: true,
  disableDirectoryRename: true,
  disableFileCreate: true,
  disableFileDelete: true,
  disableFileOpen: true,
  disableFileRename: true,
  disableEntryMove: true,
  visibilityFilter,
};

export const PackAsset = React.memo(function PackAsset(props: ProjectItemProps) {
  const { entry } = props;
  const { assetMeta } = entry.meta;

  invariant(assetMeta, 'No asset meta');

  const { requiredContextMenuItems, renderItemChildren } = useItem(props);
  const { expanded, toggleExpanded } = useExpandablePath(entry.path, false);

  const [updating, setUpdating] = React.useState(assetMeta.manager?.data?.status === assetStatus.updating);
  React.useEffect(() => {
    setUpdating(assetMeta.manager?.data?.status === assetStatus.updating);
  }, [assetMeta.manager?.data?.status]);

  const children = renderItemChildren(visibilityFilter);

  const icon = updating
    ? rotatingRefreshIcon
    : assetIcon;

  return (
    <div className={itemsStyles.wrapper}>
      <ContextMenu
        items={requiredContextMenuItems}
        className={itemsStyles.item}
        onClick={toggleExpanded}
        title={`Imported from: ${assetMeta.manager?.data?.repoUrl || 'unknown'}`}
      >
        <div className={itemsStyles.itemIcon}>
          {icon}
        </div>
        <div className={itemsStyles.itemTitle}>
          {entry.name}
        </div>

        {assetMeta.flags.readOnly && (
          <div className={itemsStyles.itemStatus}>
            <div className={itemsStyles.itemStatusEntry}>
              readonly
            </div>
          </div>
        )}
      </ContextMenu>

      {expanded && (
        <div className={itemsStyles.children}>
          <ProjectExplorerItemContextProvider
            options={contextOptions}
          >
            {children}
          </ProjectExplorerItemContextProvider>
        </div>
      )}
    </div>
  );
});
