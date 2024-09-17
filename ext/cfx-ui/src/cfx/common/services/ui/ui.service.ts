import { defineService, useService } from '../../../base/servicesContainer';

export const IUiService = defineService<IUiService>('UiService');
export interface IUiService {
  readonly viewportWidth: number;
  readonly viewportHeight: number;
  readonly quant: number;
}

export function useUiService() {
  return useService(IUiService);
}

export interface IUiDimensions {
  viewportWidth: number;
  viewportHeight: number;
  quant: number;
}
