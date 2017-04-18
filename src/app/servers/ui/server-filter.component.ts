import { Component, OnInit, OnChanges, Output, EventEmitter } from '@angular/core';

import { Server } from '../server';

export class ServerFilters {
    public searchText: string;
    public hideEmpty = false;
    public hideFull = false;
    public maxPing = 0;

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
export class ServerFilterComponent implements OnInit, OnChanges {
    filters: ServerFilters;

    @Output()
    public filtersChanged = new EventEmitter<ServerFilters>();

    constructor() {
        const storedFilters = localStorage.getItem('filters');

        if (storedFilters) {
            this.filters = {...<ServerFilters>JSON.parse(storedFilters)};
        } else {
            this.filters = new ServerFilters();
        }
    }

    ngOnInit() {
        this.filtersChanged.emit(this.filters);
    }

    ngOnChanges() {
        this.filtersChanged.emit(this.filters);

        localStorage.setItem('filters', JSON.stringify(this.filters));
    }

    updateFilters() {
        this.ngOnChanges();
    }
}