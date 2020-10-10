import { PinConfig } from "./pins";
import { ServerFilterContainer, ServerSorting } from "./components/filters/server-filter-container";
import { master } from "./master";

export class FilterRequest {
	// public servers: [string, master.IServerData][];
	public filters: ServerFilterContainer;
	public sortOrder: ServerSorting;
	public pinConfig: PinConfig;
	public fromInteraction: boolean;
}
