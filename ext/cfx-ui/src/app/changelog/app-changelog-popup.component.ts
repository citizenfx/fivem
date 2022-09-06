import { Component, OnInit, Output, EventEmitter } from '@angular/core';
import { ChangelogService } from 'app/changelogs.service';
import { delay } from 'rxjs/operators';

@Component({
	moduleId: module.id,
	selector: 'app-changelog-popup',
	templateUrl: 'app-changelog-popup.component.html',
	styleUrls: ['app-changelog-popup.component.scss']
})
export class ChangelogPopupComponent implements OnInit {
	selectedVersion = '';

	versions: string[] = [];

	isLoading = false;

	@Output()
	closeOverlay = new EventEmitter();

	constructor(private changelogs: ChangelogService) {
		this.isLoading = changelogs.versionLoadInProgress.getValue();

		changelogs.versionLoadInProgress.pipe(delay(0)).subscribe(value => this.isLoading = value);
	}

	ngOnInit() {
		this.selectedVersion = this.versions[0];

		this.changelogs.getVersions().then(versions => {
			this.versions = versions;
			this.selectedVersion = this.versions[0];
		})

		this.changelogs.readAll();
	}

	loadVersion(version: string) {
		this.selectedVersion = version;
	}
}
