import { defineApiEndpoints } from "shared/api.protocol";

export namespace FsImporterApi {
  export const Endpoints = defineApiEndpoints('importer.fs', [
    'import',
  ]);

  export interface ImportRequest {
    basePath: string,
    name: string,
    sourcePath: string,
  }
}
