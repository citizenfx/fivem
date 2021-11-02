import { SystemResource } from "backend/system-resources/system-resources-constants";
import { defineApiEndpoints } from "shared/api.protocol";

export namespace ExamplesImporterApi {
  export const Endpoints = defineApiEndpoints('importer.examples', [
    'import',
  ]);

  export interface ImportRequest {
    basePath: string,
    name: string,
    exampleName: SystemResource,
  }
}
