import { AssetImportRequest } from "shared/api.requests";

export type FsAssetImportRequest = AssetImportRequest<{ sourcePath: string }>;
