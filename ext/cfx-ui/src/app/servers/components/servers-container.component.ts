import { Component, OnInit, PLATFORM_ID, Inject, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { Server, ServerIcon } from '../server';
import { PinConfigCached } from '../pins';
import { ServersService } from '../servers.service';
import { ServerFilterContainer } from './filters/server-filter-container';

import { GameService } from '../../game.service';

import { isPlatformBrowser } from '@angular/common';

import 'rxjs/add/operator/bufferTime';
import { FiltersService } from '../filters.service';
import { bufferTime } from 'rxjs/operators';

@Component({
	moduleId: module.id,
	selector: 'servers-container',
	templateUrl: 'servers-container.component.html',
	styleUrls: ['servers-container.component.scss'],
	changeDetection: ChangeDetectionStrategy.OnPush
})
export class ServersContainerComponent implements OnInit {
	servers: { [addr: string]: Server } = {};

	localServers: Server[]; // temp value
	icons: ServerIcon[];

	pinConfig: PinConfigCached;

	filters: ServerFilterContainer;

	type: string;

	sortOrder: string[];

	constructor(
		private serverService: ServersService,
		private gameService: GameService,
		private route: ActivatedRoute,
		@Inject(PLATFORM_ID) private platformId: any,
		private cdr: ChangeDetectorRef,
		private filtersService: FiltersService,
	) {
		this.filters = new ServerFilterContainer();
		this.pinConfig = new PinConfigCached(null);
	}

	serversArray: Server[] = [];
	serverAddrToIndexMap: { [key: string]: number } = {};

	ngOnInit() {
		this.type = this.route.snapshot.data.type;

		this.filtersService.setType(this.type);

		if (isPlatformBrowser(this.platformId)) {
			requestAnimationFrame(() => {
				this.loadServers();
			});
		}

		this.serverService.rawServers = this.servers;
	}

	isBrowser() {
		return isPlatformBrowser(this.platformId);
	}

	debouncer: any = null;

	loadServers() {
		this.serverService.loadPinConfig()
			.then(pinConfig => {
				this.pinConfig = new PinConfigCached(pinConfig);
				this.filtersService.setPinConfig(pinConfig);
			});


		const typedServers = this.serverService
			.getReplayedServers()
			.filter(a => a && this.gameService.isMatchingServer(this.type, a))
			.pipe(bufferTime(1000));

		// add each new server to our server list
		typedServers.subscribe((servers) => {
			if (servers.length === 0) {
				return;
			}

			servers.forEach((server) => {
				if (!this.servers[server.address]) {
					this.serversArray.push(server);
					this.serverAddrToIndexMap[server.address] = this.serversArray.length - 1;
				} else {
					this.serversArray[this.serverAddrToIndexMap[server.address]] = server;
				}

				this.servers[server.address] = server;
			});

			this.serversArray = this.serversArray.slice();
			this.filtersService.servers = this.serversArray;

			this.cdr.markForCheck();
			this.filtersService.sortAndFilterServers(false);
		});
	}
}
