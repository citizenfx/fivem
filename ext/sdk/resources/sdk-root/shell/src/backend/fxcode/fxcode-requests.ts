import { apiEndpointsGroup } from "shared/api.protocol";

export const fxcodeApi = apiEndpointsGroup({
  getImportConfig: 'fxcode:getImportConfig',
  applyImportConfig: 'fxcode:applyImportConfig',
});
