
export class ServerFilters {
    public searchText: string;
    public hideEmpty = false;
    public hideFull = false;
	public capPing = false;
    public maxPing = 0;

    constructor() {
        this.searchText = '';
    }
}

export class ServerTags {
    public tagList: {[key: string]: boolean};
    public localeList: {[key: string]: boolean};
}

export class ServerFilterContainer {
    public filters: ServerFilters;
    public tags: ServerTags;
}
