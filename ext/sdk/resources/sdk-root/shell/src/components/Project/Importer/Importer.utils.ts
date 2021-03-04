export const inferAssetName = (sourcePath: string): string => {
  const [assetName] = sourcePath
    .split('\\')
    .map((part) => part.split('/'))
    .flat()
    .reverse();

  return assetName.replace('.git', '');
};
