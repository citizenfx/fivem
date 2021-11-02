import { AssetMeta } from "shared/asset.types";

export interface IFsEntry {
  handle: string,
  name: string,
  ctime: number,
  mtime: number,
  fxmeta?: AssetMeta | undefined,
  isDirectory: boolean,
  childrenScanned: boolean,
  children: Record<string, IFsEntry>,
}

export enum FsUpdateKind {
  Set,
  Update,
  Delete,
  Rename,
}

export type IFsUpdate =
  | { kind: FsUpdateKind.Set, parentPath: string[], fsEntry: IFsEntry }
  | { kind: FsUpdateKind.Update, path: string[], fsEntry: Partial<IFsEntry> }
  | { kind: FsUpdateKind.Rename, newParentPath: string[], oldParentPath: string[], oldName: string, newName: string }
  | { kind: FsUpdateKind.Delete, parentPath: string[], name: string };

export enum ProjectVariableSetter {
  SERVER_ONLY = 'set',
  INFORMATION = 'sets',
  REPLICATED = 'setr',
}

export type AssetName = string;

export interface IProjectVariable {
  setter: ProjectVariableSetter,
  value: string | number | boolean,
}

export enum ConvarKind {
  Bool = 'CV_BOOL',
  Int = 'CV_INT',
  Slider = 'CV_SLIDER',
  Combi = 'CV_COMBI',
  Multi = 'CV_MULTI',
  String = 'CV_STRING',
  Password = 'CV_PASSWORD',
}

export type IConvarEntry =
  | { kind: ConvarKind.Bool, title: string, setter: ProjectVariableSetter, variable: string, defaultValue: boolean }
  | { kind: ConvarKind.Int, title: string, setter: ProjectVariableSetter, variable: string, defaultValue: number, minValue: number | undefined, maxValue: number | undefined }
  | { kind: ConvarKind.Slider, title: string, setter: ProjectVariableSetter, variable: string, defaultValue: number, minValue: number | undefined, maxValue: number | undefined }
  | { kind: ConvarKind.Combi, title: string, setter: ProjectVariableSetter, variable: string, defaultValue: number, minValue: number | undefined, maxValue: number | undefined }
  | { kind: ConvarKind.Multi, title: string, setter: ProjectVariableSetter, variable: string, entries: string[] }
  | { kind: ConvarKind.String, title: string, setter: ProjectVariableSetter, variable: string, defaultValue: string }
  | { kind: ConvarKind.Password, title: string, setter: ProjectVariableSetter, variable: string, defaultValue: string };

export type IConvarCategory = { title: string, subtitle: string, entries: IConvarEntry[] };
export type IConvarCategoryMap = { [key: string]: IConvarCategory };

export type ProjectPathsState = {
  [path: string]: boolean,
};
