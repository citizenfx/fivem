import { Component, OnInit, PLATFORM_ID, Inject, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { isPlatformBrowser } from '@angular/common';

import 'rxjs/add/operator/bufferTime';
import { FiltersService } from '../filters.service';
import { ServersService } from '../servers.service';
import { GameService } from 'app/game.service';

@Component({
	moduleId: module.id,
	selector: 'servers-container',
	templateUrl: 'servers-container.component.html',
	styleUrls: ['servers-container.component.scss'],
	changeDetection: ChangeDetectionStrategy.OnPush
})
export class ServersContainerComponent implements OnInit {
	type: string;

	serversLoaded = false;
	sortingComplete = false;

	constructor(
		private route: ActivatedRoute,
		@Inject(PLATFORM_ID) private platformId: any,
		private filtersService: FiltersService,
		private serversService: ServersService,
        private gameService: GameService,
		private changeDetectorRef: ChangeDetectorRef,
	) {
		this.serversService.serversLoadedUpdate.subscribe((loaded) => {
			this.serversLoaded = loaded;
			this.changeDetectorRef.markForCheck();
		});

		this.filtersService.sortingInProgress.subscribe((inProgress) => {
			this.sortingComplete = !inProgress;
			this.changeDetectorRef.markForCheck();
		});
	}

	ngOnInit() {
		this.type = this.route.snapshot.data.type;

		this.filtersService.setType(this.type);
	}

	isBrowser() {
		return isPlatformBrowser(this.platformId);
	}

	isFiltering() {
        if (this.gameService.gameName === 'rdr3') {
            return true;
        }

		return (this.filtersService.filters?.searchText ?? '') !== '';
	}

	isLoading() {
		return (!this.serversLoaded || !this.sortingComplete);
	}
}
