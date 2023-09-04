import { PreviewBackgroundResponse } from "cfx/apps/mpMenu/services/serverPreview/serverPreview.service";
import { defineService, useService } from "cfx/base/servicesContainer";
import { ObservableAsyncValue } from "cfx/utils/observable";

export const IServerPreview = defineService<IServerPreview>('ServerPreview');
export interface IServerPreview {
  readonly currentLoadComplete: boolean;
  readonly previewBackgrounds: PreviewBackgroundResponse[]| null;

  getPreviewBackgrounds();
}

export function useServerPreviewService() {
  return useService(IServerPreview);
}