import { defineService } from "cfx/base/servicesContainer";
import { ObservableAsyncValue } from "cfx/utils/observable";

export const IServerPreview = defineService<IServerPreview>('ServerPreview');
export interface IServerPreview {
  getPreviewBackgrounds();
}
