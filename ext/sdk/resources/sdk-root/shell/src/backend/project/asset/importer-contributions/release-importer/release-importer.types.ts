import { APIRQ } from "shared/api.requests";

export type ReleaseAssetImportRequest = APIRQ.AssetImport<{ releaseUrl: string }>;
