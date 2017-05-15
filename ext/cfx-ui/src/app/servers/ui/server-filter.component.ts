import { Component, OnInit, OnChanges, Input, Output, EventEmitter } from '@angular/core';

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
    filters: ServerFilters = new ServerFilters();

    @Input()
    type: string;

    lastType = "";

    @Output()
    public filtersChanged = new EventEmitter<ServerFilters>();

    constructor() {

    }

    ngOnInit() {
        this.filtersChanged.emit(this.filters);
    }

    ngOnChanges() {
        if (this.type != this.lastType)
        {
            const storedFilters = localStorage.getItem(`sfilters:${this.type}`);

            if (storedFilters) {
                this.filters = {...<ServerFilters>JSON.parse(storedFilters)};
            } else {
                this.filters = new ServerFilters();
            }

            this.lastType = this.type;
        }

        this.filtersChanged.emit(this.filters);

        localStorage.setItem(`sfilters:${this.type}`, JSON.stringify(this.filters));
    }

    updateFilters() {
        this.ngOnChanges();
    }
}