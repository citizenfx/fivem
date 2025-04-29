import { IServerPreview } from "cfx/common/services/servers/serverPreview.service";
import { inject, injectable } from "inversify";
import { IDiscourseService } from "../discourse/discourse.service";
import { ServicesContainer } from "cfx/base/servicesContainer";
import { timeout } from "cfx/utils/async";
import { AppContribution, registerAppContribution } from "cfx/common/services/app/app.extensions";
import { makeAutoObservable } from "mobx";

export function registerServerPreviewService(container: ServicesContainer) {
    container.registerImpl(IServerPreview, ServerPreviewService);
    registerAppContribution(container, ServerPreviewService);
}

@injectable()
export class ServerPreviewService implements IServerPreview, AppContribution {
    private _currentLoadComplete: boolean = false;
    public get currentLoadComplete(): boolean { return this._currentLoadComplete }
    private set currentLoadComplete(currentLoadComplete: boolean) { this._currentLoadComplete = currentLoadComplete }

    private _previewBackgrounds: PreviewBackgroundResponse[] | null = null;
    public get previewBackgrounds(): PreviewBackgroundResponse[] | null { return this._previewBackgrounds }
    private set previewBackgrounds(previewBackgrounds: PreviewBackgroundResponse[] | null) { this._previewBackgrounds = previewBackgrounds }

    constructor(@inject(IDiscourseService) 
    protected readonly discourseService: IDiscourseService) {
        makeAutoObservable(this);
        this.getPreviewBackgrounds();
    }

    async getPreviewBackgrounds() {
        this.currentLoadComplete = false;
        this.previewBackgrounds = await this.discourseService.makeExternalCall('https://622f12cd3ff58f023c14f48c.mockapi.io/api/getBackgrounds', 'GET');
        await timeout(3000);
        this.currentLoadComplete = true;
    }

    init() {

    }
}

export type PreviewBackgroundResponse = { url: string }
