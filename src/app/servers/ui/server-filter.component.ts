import { Component, OnChanges, Output, EventEmitter } from '@angular/core';

import { Server } from '../server';

export class ServerFilters {
    public searchText: string;

    constructor() {
        this.searchText = '';
    }
}

@Component({
    moduleId: module.id,
    selector: 'app-server-filter',
    templateUrl: 'server-filter.component.html',
    styleUrls: ['server-filter.component.scss']
})
export class ServerFilterComponent implements OnChanges {
    private filters: ServerFilters;

    @Output()
    public filtersChanged = new EventEmitter<ServerFilters>();

    constructor() {
        this.filters = new ServerFilters();
    }

    ngOnChanges() {
        this.filtersChanged.emit(this.filters);
    }

    updateFilters() {
        this.ngOnChanges();
    }
}