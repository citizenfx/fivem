import { APIRQ } from "shared/api.requests";

export type GitAssetImportRequest = APIRQ.AssetImport<{ repoUrl: string }>;
