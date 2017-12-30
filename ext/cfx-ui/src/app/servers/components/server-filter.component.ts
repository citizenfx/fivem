import { Component, OnInit, OnChanges, Input, Output, EventEmitter } from '@angular/core';

import { Server } from '../server';

import { ServersService } from '../servers.service';

import { Subject } from 'rxjs/Subject';

import 'rxjs/add/operator/throttleTime';

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

    isRefreshing = false;
    wantsToBeRefreshing = false;

    refreshEvent = new Subject<void>();

	public mouseState = false;
	public maxPingPercent = 0;
	private minPingLimit = 30;
    private maxPingLimit = 200;
    
    constructor(private serversService: ServersService) {
        this.serversService
            .getServers()
            .filter(server => !server)
            .subscribe(server => this.isRefreshing = false);

        this.refreshEvent
            .throttleTime(10000)
            .subscribe(() => { this.wantsToBeRefreshing = false; this.isRefreshing = true; this.serversService.refreshServers() });
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

		let pingBar = document.getElementById("progress").clientWidth;
		let widthPercent = (this.filters.maxPing-this.minPingLimit)/(this.maxPingLimit-this.minPingLimit);
		this.maxPingPercent = widthPercent*100;
		
        this.filtersChanged.emit(this.filters);
        localStorage.setItem(`sfilters:${this.type}`, JSON.stringify(this.filters));
    }

    updateFilters() {
        this.ngOnChanges();
    }
	
	isPingCap() {
		return this.filters.capPing;
	}
	
	changeMaxPing($event, force) {
		if ( this.mouseState || force ) {
			let pingBar = document.getElementById("progress").clientWidth;
			let widthPercent = $event.offsetX/pingBar;
			let ping = (widthPercent*(this.maxPingLimit-this.minPingLimit))+this.minPingLimit;
			
			this.maxPingPercent = widthPercent*100;
			this.filters.maxPing = Math.floor(ping);
			this.filtersChanged.emit(this.filters);
		}
    }
    
    refresh() {
        if (!this.wantsToBeRefreshing) {
            // comforting timeout for spam clicks, no debouncing though
            window.setTimeout(() => {
                this.wantsToBeRefreshing = false;
            }, 1000);

            this.wantsToBeRefreshing = true;
        }

        this.refreshEvent.next();
    }
}