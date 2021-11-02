import { defineApiEndpoints } from "shared/api.protocol";

export namespace GitImporterApi {
  export const Endpoints = defineApiEndpoints('importer.git', [
    'importRepository',
    'importRelease',
  ]);

  export interface ImportRepositoryRequest {
    basePath: string,
    name: string,
    repositoryUrl: string,
  }

  export interface ImportGithubReleaseRequest {
    basePath: string,
    name: string,
    releaseUrl: string,
  }
}
