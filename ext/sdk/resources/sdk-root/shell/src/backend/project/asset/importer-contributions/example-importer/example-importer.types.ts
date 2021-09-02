import { SystemResource } from "backend/system-resources/system-resources-constants";
import { APIRQ } from "shared/api.requests";

export type ExampleAssetImportRequest = APIRQ.AssetImport<{ exampleName: SystemResource }>;
