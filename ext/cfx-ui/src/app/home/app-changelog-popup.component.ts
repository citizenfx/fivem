import { Component, OnInit, ElementRef, ViewChild, Input, Output, EventEmitter } from '@angular/core';
import { ChangelogService } from 'app/changelogs.service';

@Component({
	moduleId:    module.id,
	selector:    'app-changelog-popup',
	templateUrl: 'app-changelog-popup.component.html',
	styleUrls:   ['app-changelog-popup.component.scss']
})
export class ChangelogPopupComponent implements OnInit {
	selectedVersion = '';

	versions: string[] = [
		'2069',
		'2042'
	];

	@Output()
	closeOverlay = new EventEmitter();

	constructor(private changelogs: ChangelogService) {

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
