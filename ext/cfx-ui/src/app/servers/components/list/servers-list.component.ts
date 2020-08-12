import {
	Component, OnChanges, Input, NgZone, Inject, PLATFORM_ID, ChangeDetectorRef,
	ChangeDetectionStrategy, ElementRef, ViewChild, AfterViewInit, OnInit
} from '@angular/core';
import { Server, ServerHistoryEntry } from '../../server';
import { PinConfigCached } from '../../pins';
import { Subject } from 'rxjs/Subject';

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
export class ServersListComponent implements OnInit, OnChanges, AfterViewInit {
	@Input()
	private servers: Server[];

	@Input()
	private pinConfig: PinConfigCached;

	@Input()
	type: string;

	private subscriptions: { [addr: string]: any } = {};

	private lastLength: number;

	@ViewChild('list')
	private list: ElementRef;

	private interactingUntil = 0;

	HistoryServerStatus = HistoryServerStatus;

	localServers: Server[];
	sortedServers: Server[] = [];
	historyServers: HistoryServer[] = [];

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
		this.servers = [];

		let changed = false;

		this.changeObservable.subscribe(() => {
			changed = true;
		});

		this.filtersService.sortedServersUpdate.subscribe((servers) => {
			this.sortedServers = servers;
			this.changeDetectorRef.markForCheck();
		});

		zone.runOutsideAngular(() => {
			setInterval(() => {
				if (changed) {
					if (this.interactingUntil >= new Date().getTime()) {
						return;
					}

					changed = false;

					for (const server of (this.servers || [])) {
						if (!this.subscriptions[server.address]) {
							this.subscriptions[server.address] = server.onChanged.subscribe(a => this.changeSubject.next());
						}
					}
				}
			}, 500);
		});
	}

	isBrowser() {
		return isPlatformBrowser(this.platformId);
	}

	isPinned(server: Server) {
		if (!this.pinConfig || !this.pinConfig.pinnedServers) {
			return false;
		}

		return this.pinConfig.pinnedServers.has(server.address);
	}

	isPremium(server: Server) {
		return !!server.premium;
	}

	getPremium(server: Server) {
		return server.premium;
	}

	// to prevent auto-filtering while scrolling (to make scrolling feel smoother)
	updateInteraction() {
		this.interactingUntil = new Date().getTime() + 500;
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
		}
	}

	attemptConnectTo(entry: HistoryServer) {
		if (!entry.server) {
			return;
		}

		this.gameService.connectTo(entry.server, entry.historyEntry.address);
	}

	ngAfterViewInit() {
		const element = this.list.nativeElement as HTMLElement;

		this.zone.runOutsideAngular(() => {
			element.addEventListener('wheel', (e) => {
				this.updateInteraction();
			});
		});
	}

	changeSubject: Subject<void> = new Subject<void>();
	changeObservable = this.changeSubject.asObservable();

	ngOnChanges() {
		if (this.servers.length !== this.lastLength) {
			this.changeSubject.next();
			this.lastLength = this.servers.length;
		}
	}

	svTrack(index: number, serverRow: Server) {
		return serverRow.address;
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
