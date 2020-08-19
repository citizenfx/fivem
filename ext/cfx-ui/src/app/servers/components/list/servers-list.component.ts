import {
	Component, Input, NgZone, Inject, PLATFORM_ID, ChangeDetectorRef,
	ChangeDetectionStrategy, ElementRef, ViewChild, OnInit, OnDestroy
} from '@angular/core';
import { Server } from '../../server';
import { PinConfigCached } from '../../pins';

import { isPlatformBrowser } from '@angular/common';

import 'rxjs/add/operator/throttleTime';
import { ServersService, HistoryServerStatus, HistoryServer } from '../../servers.service';
import { FiltersService } from '../../filters.service';
import { GameService } from 'app/game.service';
import { DomSanitizer } from '@angular/platform-browser';
import { L10nLocale, L10N_LOCALE } from 'angular-l10n'

@Component({
	moduleId: module.id,
	selector: 'servers-list',
	templateUrl: 'servers-list.component.html',
	styleUrls: ['servers-list.component.scss'],
	changeDetection: ChangeDetectionStrategy.OnPush
})
export class ServersListComponent implements OnInit, OnDestroy {
	@Input()
	private pinConfig: PinConfigCached;

	@Input()
	type: string;

	private subscriptions: { [addr: string]: any } = {};

	@ViewChild('list')
	private list: ElementRef;

	HistoryServerStatus = HistoryServerStatus;

	serversLoaded = false;
	sortingComplete = false;

	sortedServers: Server[] = [];
	historyServers: HistoryServer[] = [];

	private sortedServersUpdaterTimer;

	private serversBatchRenderingAmount = 10;

	constructor(
		private zone: NgZone,
		@Inject(PLATFORM_ID) private platformId: any,
		public changeDetectorRef: ChangeDetectorRef,
		private serversService: ServersService,
		private filtersService: FiltersService,
		private gameService: GameService,
		private sanitizer: DomSanitizer,
		@Inject(L10N_LOCALE) public locale: L10nLocale,
	) {
		this.serversService.serversLoadedUpdate.subscribe((loaded) => {
			this.serversLoaded = loaded;
		});

		this.filtersService.sortingInProgress.subscribe((inProgress) => {
			this.sortingComplete = !inProgress;
		});

		this.filtersService.sortedServersUpdate.subscribe((servers) => {
			if (this.sortedServersUpdaterTimer) {
				cancelAnimationFrame(this.sortedServersUpdaterTimer);
			}

			if (servers.length <= this.serversBatchRenderingAmount) {
				this.sortedServers = servers;
				this.changeDetectorRef.markForCheck();
				return;
			}

			this.sortedServers = [];
			let currentServerIndex = 0;

			const renderServersBatch = () => {
				// If we have a sorting started by filters change - cancel further incremental rendering
				if (!this.sortingComplete) {
					this.sortedServers = [];
					this.sortedServersUpdaterTimer = null;
					this.changeDetectorRef.markForCheck();
					return;
				}

				// Rendering servers in batches to improme initial render time
				for (let i = 0; i < this.serversBatchRenderingAmount; i++) {
					this.sortedServers.push(servers[currentServerIndex]);

					currentServerIndex++;

					if (currentServerIndex === servers.length - 1)  {
						this.sortedServersUpdaterTimer = null;
						return;
					}
				}

				this.sortedServersUpdaterTimer = requestAnimationFrame(renderServersBatch);
			};

			this.sortedServersUpdaterTimer = requestAnimationFrame(renderServersBatch);
			this.changeDetectorRef.markForCheck();
		});
	}

	get shouldShowHint() {
		if (this.type !== 'history' && this.type !== 'favorites') {
			return false;
		}

		if (!this.serversLoaded || !this.sortingComplete) {
			return false;
		}

		return this.sortedServers.length === 0;
	}

	get hintText() {
		switch (this.type) {
			case 'history': return '#EmptyServers_History';
			case 'favorites': return '#EmptyServers_Favorites';
		}
	}

	ngOnDestroy() {
		if (this.sortedServersUpdaterTimer) {
			cancelAnimationFrame(this.sortedServersUpdaterTimer);
		}
	}

	isBrowser() {
		return isPlatformBrowser(this.platformId);
	}

	isPinned(server: Server) {
		return this.filtersService.pinConfig.pinnedServers.has(server?.address);
	}

	isPremium(server: Server) {
		return !!server.premium;
	}

	getPremium(server: Server) {
		return server.premium;
	}

	ngOnInit() {
		if (this.type === 'history') {
			this.historyServers = this.gameService
				.getServerHistory()
				.slice(-6)
				.map((historyEntry) => ({
					historyEntry,
					server: null,
					sanitizedIcon: (historyEntry.icon ? this.sanitizer.bypassSecurityTrustUrl(historyEntry.icon) : null),
					status: HistoryServerStatus.Loading,
				}))
				.reverse();

			this.historyServers.forEach(async (historyServer) => {
				const isAddressServer = historyServer.historyEntry.address.includes(':');

				if (!isAddressServer) {
					try {
						historyServer.server = await this.serversService.getServer(historyServer.historyEntry.address);
						historyServer.status = HistoryServerStatus.Online;
					} catch (e) {
						historyServer.status = HistoryServerStatus.Offline;
					}
				} else {
					try {
						historyServer.server = await this.gameService.queryAddress(
							this.serversService.parseAddress(historyServer.historyEntry.address),
						);

						historyServer.status = HistoryServerStatus.Online;
					} catch (e) {
						historyServer.status = HistoryServerStatus.Offline;
					}
				}

				this.changeDetectorRef.markForCheck();
			});
		} else {
			this.filtersService.sortAndFilterServers(false);
		}
	}

	attemptConnectTo(entry: HistoryServer) {
		if (!entry.server) {
			return;
		}

		this.gameService.connectTo(entry.server, entry.historyEntry.address);
	}

	svTrack(index: number, serverRow: Server) {
		return index + serverRow.address;
	}

	isFavorite(server: Server) {
		return this.gameService.isMatchingServer('favorites', server);
	}

	toggleFavorite(event, server: Server) {
		if (this.isFavorite(server)) {
			this.gameService.toggleListEntry('favorites', server, false);
		} else {
			this.gameService.toggleListEntry('favorites', server, true);
		}

		event.stopPropagation();
	}
}
