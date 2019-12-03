import { PinConfig } from "./pins";
import { ServerFilterContainer } from "./components/server-filter-container";
import { master } from "./master";

export class FilterRequest {
    public servers: [ string, master.IServerData ][];
    public filters: ServerFilterContainer;
    public sortOrder: string[];
    public pinConfig: PinConfig;
    public fromInteraction: boolean;
}