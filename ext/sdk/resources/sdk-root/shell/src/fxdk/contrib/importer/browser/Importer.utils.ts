import { FsBrowserUtils } from "fxdk/project/contrib/explorer/ui/FsBrowser/FsBrowser.utils";

export const inferAssetName = (sourcePath: string): string => {
  const [assetName] = sourcePath
    .split('\\')
    .map((part) => part.split('/'))
    .flat()
    .reverse();

  return assetName.replace('.git', '');
};

export const defaultAssetImportPathSelectableFilter = FsBrowserUtils.combineSelectableFilters(
  FsBrowserUtils.filters.discardFiles,
  FsBrowserUtils.filters.discardDotFilesAndDirs,
  FsBrowserUtils.filters.discardAssets,
);
