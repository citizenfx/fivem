import { AssetImportRequest } from "shared/api.requests";

export type GitAssetImportRequest = AssetImportRequest<{ repoUrl: string }>;
