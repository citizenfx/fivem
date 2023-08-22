import { IServerPreview } from "cfx/common/services/servers/serverPreview.service";
import { inject, injectable } from "inversify";
import { IDiscourseService } from "../discourse/discourse.service";
import { ServicesContainer, useService } from "cfx/base/servicesContainer";

export function registerServerPreviewService(container: ServicesContainer) {
    container.registerImpl(IServerPreview, ServerPreviewService);
}

export function useServerPreviewService() {
    return useService(ServerPreviewService);
}

@injectable()
export class ServerPreviewService implements IServerPreview {
    private _currentLoadComplete: boolean = false;
    public get currentLoadComplete(): boolean { return this._currentLoadComplete }
    private set currentLoadComplete(currentLoadComplete: boolean) { this._currentLoadComplete = currentLoadComplete }

    private _previewBackgrounds: PreviewBackgroundResponse[] | null = null;
    public get previewBackgrounds(): PreviewBackgroundResponse[] | null { return this._previewBackgrounds }
    private set previewBackgrounds(previewBackgrounds: PreviewBackgroundResponse[] | null) { this._previewBackgrounds = previewBackgrounds }

    constructor(@inject(IDiscourseService)
    protected readonly discourseService: IDiscourseService) {}
    async getPreviewBackgrounds() {
        this.currentLoadComplete = false;
        this.previewBackgrounds = await this.discourseService.makeExternalCall('https://622f12cd3ff58f023c14f48c.mockapi.io/api/getBackgrounds', 'GET');
    }
}

type PreviewBackgroundResponse = { url: string }
