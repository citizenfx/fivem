import { SystemResource } from "backend/system-resources/system-resources-constants";
import { AssetImportRequest } from "shared/api.requests";

export type ExampleAssetImportRequest = AssetImportRequest<{ exampleName: SystemResource }>;
