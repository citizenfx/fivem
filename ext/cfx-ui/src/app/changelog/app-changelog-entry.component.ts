import { Component, OnInit, Input, OnChanges } from '@angular/core';
import { ChangelogService } from 'app/changelogs.service';
import { GameService } from 'app/game.service';

import { DomSanitizer, SafeHtml } from '@angular/platform-browser';

@Component({
	moduleId: module.id,
	selector: 'app-changelog-entry',
	templateUrl: 'app-changelog-entry.component.html',
	styleUrls: ['app-changelog-entry.component.scss']
})
export class ChangelogEntryComponent implements OnInit, OnChanges {
	@Input()
	version = '';
	lastVersion = '';

	text: SafeHtml | string = '';

	constructor(private changelog: ChangelogService, private gameService: GameService, private sanitizer: DomSanitizer) {

	}

	ngOnInit() {
		this.fetchVersion();
	}

	ngOnChanges() {
		if (this.version !== this.lastVersion) {
			this.fetchVersion();
		}
	}

	private async fetchVersion() {
		this.lastVersion = this.version;

		this.text = this.sanitizer.bypassSecurityTrustHtml(await this.changelog.getVersion(this.version));
	}

	clickContent(event: MouseEvent) {
		const srcElement = event.srcElement as HTMLElement;

		if (srcElement.localName === 'a') {
			this.gameService.openUrl(srcElement.getAttribute('href'));

			event.preventDefault();
		}
	}
}
