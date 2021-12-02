import { defineApiEndpoints } from "shared/api.protocol";

export namespace FXWorldApi {
  export const Endpoints = defineApiEndpoints('assets.fxworld', [
    'create',
  ]);

  export interface CreateRequest {
    basePath: string,
    name: string,
  }
}
