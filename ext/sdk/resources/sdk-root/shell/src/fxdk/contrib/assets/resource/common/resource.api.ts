import { defineApiEndpoints } from "shared/api.protocol";

export namespace ResourceApi {
  export const Endpoints = defineApiEndpoints('assets.resource', [
    'create',
  ]);

  export interface CreateRequest {
    basePath: string,
    name: string,
    resourceTemplateId?: string,
  }
}
