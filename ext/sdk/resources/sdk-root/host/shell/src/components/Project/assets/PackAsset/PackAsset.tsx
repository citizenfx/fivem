import React from 'react';
import classnames from 'classnames';
import { combineVisibilityFilters, visibilityFilters } from '../../../Explorer/Explorer';
import { ProjectItemProps } from '../../ResourceExplorer/ResourceExplorer.item';
import { useOpenFlag } from '../../../../utils/hooks';
import { assetStatus } from '../../../../sdkApi/api.types';
import { invariant } from '../../../../utils/invariant';
import { assetIcon, rotatingRefreshIcon } from '../../../../constants/icons';
import { DirectoryContextProdiver } from '../../ResourceExplorer/Directory/Directory.context';
import { ResourceContextProvider } from '../../ResourceExplorer/Resource/Resource.context';
import s from './PackAsset.module.scss';
import { useExpandablePath } from '../../ResourceExplorer/ResourceExplorer.hooks';


const visibilityFilter = combineVisibilityFilters(
  visibilityFilters.hideDotFilesAndDirs,
  visibilityFilters.hideFiles,
);

export const PackAsset = React.memo((props: ProjectItemProps) => {
  const { entry, pathsMap, itemRenderer } = props;
  const { assetMeta } = entry.meta;

  invariant(assetMeta, 'No asset meta');

  const { expanded, toggleExpanded } = useExpandablePath(entry.path, false);

  const [updating, setUpdating] = React.useState(entry.meta.assetMeta?.manager?.data?.status === assetStatus.updating);
  React.useEffect(() => {
    setUpdating(entry.meta.assetMeta?.manager?.data?.status === assetStatus.updating);
  }, [entry.meta.assetMeta?.manager?.data?.status]);

  const children = (pathsMap[entry.path] || [])
    .filter(visibilityFilter)
    .map((child) => itemRenderer({
      ...props,
      entry: child,
    }));

  const rootClassName = classnames(s.root, {
    [s.open]: expanded,
  });

  const icon = updating
    ? rotatingRefreshIcon
    : assetIcon;

  return (
    <div className={rootClassName}>
      <div
        className={s.name}
        onClick={toggleExpanded}
        title={`Imported from: ${entry.meta.assetMeta?.manager?.data?.repoUrl || 'unknown'}`}
      >
        {icon}
        {entry.name}

        {entry.meta.assetMeta?.flags.readOnly && (
          <div className={s.readonly}>
            <span>readonly</span>
          </div>
        )}
      </div>

      {expanded && (
        <div className={s.children}>
          <DirectoryContextProdiver
            forbidCreateDirectory
            forbidCreateResource
            forbidDeleteDirectory
            visibilityFilter={visibilityFilter}
          >
            <ResourceContextProvider
              forbidDeletion
              forbidRenaming
            >
              {children}
            </ResourceContextProvider>
          </DirectoryContextProdiver>
        </div>
      )}
    </div>
  );
});
