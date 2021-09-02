import { APIRQ } from "shared/api.requests";

export type FsAssetImportRequest = APIRQ.AssetImport<{ sourcePath: string }>;
