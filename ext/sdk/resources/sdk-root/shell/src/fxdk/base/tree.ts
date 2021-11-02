import { IDisposableObject } from "./disposable";

export interface ITreeLeaf<ItemType> {
  name: string,
  item: ItemType | undefined,

  /**
   * Only null if is root leaf
   */
  parent: ITreeLeaf<ItemType> | null,
  children: Record<string, ITreeLeaf<ItemType>>,
}

export type ITreeLeafTraverser<ItemType> = (leaf: ITreeLeaf<ItemType>, fullItemPath: string) => void | Promise<void>;

export type ITreeOptionalItemTraverser<ItemType> = (item: ItemType | undefined, fullItemPath: string) => void | Promise<void>;
export type ITreeItemTraverser<ItemType> = (item: ItemType | undefined, fullItemPath: string) => void | Promise<void>;

export class Tree<ItemType> implements IDisposableObject {
  protected rootLeafs: Record<string, ITreeLeaf<ItemType>> = Object.create(null);

  protected pathsMap: Record<string, ItemType> = Object.create(null);
  protected reversePathsMap: Map<ItemType, string> = new Map();

  constructor(
    public readonly rootPath: string,
    public readonly separator: string,
  ) {}

  dispose() {
    this.traverseAll(this.boundDeleteLeafItem);
    this.rootLeafs = Object.create(null);
    this.pathsMap = Object.create(null);
    this.reversePathsMap.clear();
  }

  get(fullItemPath: string): ItemType | undefined {
    return this.pathsMap[fullItemPath];
  }

  add(fullItemPath: string, item: ItemType) {
    const leaf = this.getOrCreateLeaf(fullItemPath);

    leaf.item = item;

    this.pathsMap[fullItemPath] = item;
    this.reversePathsMap.set(item, fullItemPath);
  }

  prune(fullItemPath: string) {
    const leaf = this.getLeaf(fullItemPath);
    if (!leaf) {
      return;
    }

    this.traverseLeaf(leaf, this.boundDeleteLeafItem, fullItemPath);

    if (leaf.parent) {
      delete leaf.parent.children[leaf.name];
    } else {
      delete this.rootLeafs[leaf.name];
    }
  }

  delete(fullItemPath: string) {
    const leaf = this.getLeaf(fullItemPath);
    if (!leaf) {
      return;
    }

    this.deleteLeafItem(leaf, fullItemPath);
  }

  deleteDeep(fullItemPath: string) {
    const leaf = this.getLeaf(fullItemPath);
    if (!leaf) {
      return;
    }

    this.traverseLeaf(leaf, this.boundDeleteLeafItem, fullItemPath);
  }

  relocate(fromFullItemPath: string, toFullItemPath: string): boolean {
    const fromLeaf = this.getLeaf(fromFullItemPath);
    if (!fromLeaf) {
      return false;
    }

    if (fromLeaf.parent) {
      delete fromLeaf.parent.children[fromLeaf.name];
    } else {
      delete this.rootLeafs[fromLeaf.name];
    }

    const toLeaf = this.getOrCreateLeaf(toFullItemPath);

    toLeaf.item = fromLeaf.item;
    toLeaf.children = fromLeaf.children;

    for (const childLeaf of Object.values(toLeaf.children)) {
      childLeaf.parent = toLeaf;
    }

    // Update paths for items in maps
    this.traverseLeaf(toLeaf, (leaf, fullLeafPath) => {
      if (!leaf.item) {
        return;
      }

      const oldFullLeafPath = this.reversePathsMap.get(leaf.item);
      if (!oldFullLeafPath) {
        return;
      }

      delete this.pathsMap[oldFullLeafPath];
      this.pathsMap[fullLeafPath] = leaf.item;

      this.reversePathsMap.set(leaf.item, fullLeafPath);
    }, toFullItemPath);

    return true;
  }

  getAll(): ItemType[] {
    return Object.values(this.pathsMap);
  }

  traversePath(fullItemPath: string, fn: ITreeLeafTraverser<ItemType>) {
    const itemPath = this.getPath(fullItemPath);

    let currentItemName = itemPath.shift();
    if (!currentItemName) {
      return;
    }

    let currentLeaf = this.rootLeafs[currentItemName];
    let currentLeafPath = this.joinPath(this.rootPath, currentItemName);

    while (currentLeaf) {
      fn(currentLeaf, currentLeafPath);

      currentItemName = itemPath.shift();
      if (!currentItemName) {
        return;
      }

      currentLeaf = currentLeaf.children[currentItemName];
      currentLeafPath = this.joinPath(currentLeafPath, currentItemName);
    }
  }

  traverseAll(fn: ITreeLeafTraverser<ItemType>) {
    for (const [leafName, leafChild] of Object.entries(this.rootLeafs)) {
      this.traverseLeaf(leafChild, fn, this.joinPath(this.rootPath, leafName));
    }
  }

  traverseLeaf(leaf: ITreeLeaf<ItemType>, fn: ITreeLeafTraverser<ItemType>, startPath: string) {
    for (const [leafName, leafChild] of Object.entries(leaf.children)) {
      this.traverseLeaf(leafChild, fn, this.joinPath(startPath, leafName));
    }

    // Inside-out traversing
    fn(leaf, startPath);
  }

  traverseItemsWithin(fullItemPath: string, fn: ITreeItemTraverser<ItemType>) {
    return this.traverseOptionalItemsWithin(fullItemPath, (item, fullLeafPath) => item && fn(item, fullLeafPath));
  }

  traverseOptionalItemsWithin(fullItemPath: string, fn: ITreeOptionalItemTraverser<ItemType>) {
    const leaf = this.getLeaf(fullItemPath);
    if (!leaf) {
      return;
    }

    this.traverseLeaf(leaf, (nestedLeaf, fullLeafPath) => fn(nestedLeaf.item, fullLeafPath), fullItemPath);
  }

  protected boundDeleteLeafItem = (leaf: ITreeLeaf<ItemType>, fullItemPath: string) => this.deleteLeafItem(leaf, fullItemPath);

  protected deleteLeafItem(leaf: ITreeLeaf<ItemType>, fullItemPath: string) {
    if (leaf.item) {
      delete this.pathsMap[fullItemPath];
      this.reversePathsMap.delete(leaf.item)
      leaf.item = undefined;
    }
  }

  protected getLeaf(fullItemPath: string): ITreeLeaf<ItemType> | undefined {
    return this.getLeafByPath(this.getPath(fullItemPath));
  }

  protected getParentLeaf(fullItemPath: string): ITreeLeaf<ItemType> | undefined {
    return this.getLeafByPath(this.getPathParentPath(this.getPath(fullItemPath)));
  }

  protected getLeafByPath([inRootName, ...itemPath]: string[]): ITreeLeaf<ItemType> | undefined {
    if (!inRootName) {
      return;
    }

    if (!itemPath.length) {
      return this.rootLeafs[inRootName];
    }

    return itemPath.reduce((leaf, pathPart) => leaf?.children[pathPart], this.rootLeafs[inRootName]);
  }

  protected getOrCreateLeaf(fullItemPath: string): ITreeLeaf<ItemType> {
    const [inRootName, ...itemPath] = this.getPath(fullItemPath);
    if (!inRootName) {
      throw new Error(`Cannot create tree leaf for "${fullItemPath}" when root path is "${this.rootPath}"`);
    }

    const rootLeaf = this.rootLeafs[inRootName] ??= {
      name: inRootName,
      item: undefined,
      parent: null,
      children: {},
    };

    if (!itemPath.length) {
      return rootLeaf;
    }

    return itemPath.reduce((leaf, pathPart) => leaf.children[pathPart] ??= {
      name: pathPart,
      item: undefined,
      parent: leaf,
      children: {},
    }, rootLeaf);
  }

  protected getPath(fullItemPath: string): string[] {
    return fullItemPath.substr(this.rootPath.length + this.separator.length).split(this.separator);
  }

  protected getPathName(itemPath: string[]): string {
    return itemPath[itemPath.length - 1];
  }

  protected getPathParentPath(itemPath: string[]): string[] {
    return itemPath.slice(0, -1);
  }

  protected joinPath(root: string, leafName: string): string {
    return root + this.separator + leafName;
  }

  toJSON() {
	  return {
		  path: this.rootPath,
		  pathsMap: Object.entries(this.pathsMap).reduce((json, [path, item]) => {
        json[path] = this.itemToJSON(item);

        return json;
      }, {}),
		  children: Object.entries(this.rootLeafs).reduce((json, [name, leaf]) => {
				json[name] = this.leafToJSON(leaf, this.joinPath(this.rootPath, name));

				return json;
			}, {} as any),
	  };
  }

  private leafToJSON(leaf: ITreeLeaf<ItemType>, fullLeafPath: string) {
	  return {
		  path: fullLeafPath,
      item: leaf.item
        ? this.itemToJSON(leaf.item)
        : undefined,
		  children: Object.entries(leaf.children).reduce((children, [name, leaf]) => {
			  children[name] = this.leafToJSON(leaf, this.joinPath(fullLeafPath, name));

			  return children;
		  }, {} as Record<string, any>),
	  };
  }

  protected itemToJSON(item: ItemType): any {
    return 'item';
  }
}

export class AsyncTree<ItemType> extends Tree<ItemType> {
  override async prune(fullItemPath: string): Promise<void> {
    const leaf = this.getLeaf(fullItemPath);
    if (!leaf) {
      return;
    }

    await this.traverseLeaf(leaf, this.boundDeleteLeafItem, fullItemPath);
  }

  override async delete(fullItemPath: string): Promise<void> {
    const leaf = this.getLeaf(fullItemPath);
    if (!leaf) {
      return;
    }

    return this.deleteLeafItem(leaf, fullItemPath);
  }

  override async deleteDeep(fullItemPath: string): Promise<void> {
    const leaf = this.getLeaf(fullItemPath);
    if (!leaf) {
      return;
    }

    return this.traverseLeaf(leaf, this.boundDeleteLeafItem, fullItemPath);
  }

  override async traversePath(fullItemPath: string, fn: ITreeLeafTraverser<ItemType>) {
    const itemPath = this.getPath(fullItemPath);

    let currentItemName = itemPath.shift();
    if (!currentItemName) {
      return;
    }

    let currentLeaf = this.rootLeafs[currentItemName];
    let currentLeafPath = this.joinPath(this.rootPath, currentItemName);

    while (currentLeaf) {
      await fn(currentLeaf, currentLeafPath);

      currentItemName = itemPath.shift();
      if (!currentItemName) {
        return;
      }

      currentLeaf = currentLeaf.children[currentItemName];
      currentLeafPath = this.joinPath(currentLeafPath, currentItemName);
    }
  }

  override async traverseLeaf(leaf: ITreeLeaf<ItemType>, fn: ITreeLeafTraverser<ItemType>, startPath: string): Promise<void> {
    await Promise.all(Object.entries(leaf.children).map(([leafName, leafChild]) => {
      return this.traverseLeaf(leafChild, fn, this.joinPath(startPath, leafName));
    }));

    // Inside-out traverse
    await fn(leaf, startPath);
  }

  override traverseItemsWithin(fullItemPath: string, fn: ITreeItemTraverser<ItemType>): Promise<void> {
    return this.traverseOptionalItemsWithin(fullItemPath, (item, fullLeafPath) => item && fn(item, fullLeafPath));
  }

  override async traverseOptionalItemsWithin(fullItemPath: string, fn: ITreeOptionalItemTraverser<ItemType>): Promise<void> {
    const leaf = this.getLeaf(fullItemPath);
    if (!leaf) {
      return;
    }

    await this.traverseLeaf(leaf, (nestedLeaf, fullLeafPath) => fn(nestedLeaf.item, fullLeafPath), fullItemPath);
  }

  override async traverseAll(fn: ITreeLeafTraverser<ItemType>): Promise<void> {
    for (const [leafName, leafChild] of Object.entries(this.rootLeafs)) {
      await this.traverseLeaf(leafChild, fn, this.joinPath(this.rootPath, leafName));
    }
  }
}
