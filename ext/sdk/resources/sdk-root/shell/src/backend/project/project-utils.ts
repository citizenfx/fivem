type AssetPath = string;
type AssetPathsTree = { [key: string]: AssetPathsTree | null };

export function getAssetsPriorityQueue(assetPathsUnsorted: AssetPath[]): string[][] {
  const assetPaths = assetPathsUnsorted.sort((a, b) => a.length - b.length);

  const pathsTree: AssetPathsTree = {};
  const pathsPriorities = {};

  const getMapInTree = (assetPath: string) => {
    let currentTree = pathsTree;

    if (!pathsPriorities[assetPath]) {
      pathsPriorities[assetPath] = 0;
    }

    const pathInTree: string[] = [];

    loop: while (true) {
      for (const subAssetPath of Object.keys(currentTree)) {
        if (assetPath.indexOf(subAssetPath) === 0) {
          pathInTree.push(subAssetPath);

          let subTree = currentTree[subAssetPath];

          // New sub tree forming
          if (!subTree) {
            currentTree[subAssetPath] = subTree = {};

            pathInTree.forEach((p, i) => pathsPriorities[p] = pathInTree.length - i);

            return subTree;
          } else {
            currentTree = subTree;
            continue loop;
          }
        }
      }

      pathInTree.forEach((p, i) => pathsPriorities[p] = pathInTree.length - i);
      return currentTree;
    }
  };

  // Forming tree recording priorities
  for (const assetPath of assetPaths) {
    getMapInTree(assetPath)[assetPath] = null;
  }

  return assetPaths.reduce((acc, assetPath) => {
    const priority = pathsPriorities[assetPath];

    (acc[priority] || (acc[priority] = [])).push(assetPath);

    return acc;
  }, [] as string[][]);
}
