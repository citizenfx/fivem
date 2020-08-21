import { Component, OnInit, PLATFORM_ID, Inject, ChangeDetectionStrategy } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { isPlatformBrowser } from '@angular/common';

import 'rxjs/add/operator/bufferTime';
import { FiltersService } from '../filters.service';

@Component({
	moduleId: module.id,
	selector: 'servers-container',
	templateUrl: 'servers-container.component.html',
	styleUrls: ['servers-container.component.scss'],
	changeDetection: ChangeDetectionStrategy.OnPush
})
export class ServersContainerComponent implements OnInit {
	type: string;

	constructor(
		private route: ActivatedRoute,
		@Inject(PLATFORM_ID) private platformId: any,
		private filtersService: FiltersService,
	) {
	}

	ngOnInit() {
		this.type = this.route.snapshot.data.type;

		this.filtersService.setType(this.type);
	}

	isBrowser() {
		return isPlatformBrowser(this.platformId);
	}
}
