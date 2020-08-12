import { Injectable, EventEmitter } from "@angular/core";
import { HttpClient } from "@angular/common/http";
import { BehaviorSubject } from "rxjs";

@Injectable()
export class ChangelogService {
	private targetEndpoint = 'https://changelogs-live.fivem.net/api/changelog/';

	private loadsInProgress = 0;

	onRead = new EventEmitter();

	versionLoadInProgress = new BehaviorSubject<boolean>(false);

	constructor(private http: HttpClient) {

	}

	async getUnreadItems() {
		const versions = await this.getVersions();
		const savedVersions = await this.getSavedVersions();

		if (savedVersions.size === 0) {
			return 1;
		}

		let count = 0;

		for (const version of versions) {
			if (!savedVersions.has(version)) {
				count++;
			}
		}

		return count;
	}

	async readAll() {
		const versions = await this.getVersions();
		this.addSavedVersions(versions);

		this.onRead.emit();
	}

	getSavedVersions(): Set<string> {
		return new Set<string>(JSON.parse(window.localStorage.getItem('changelogVersions') || '[]'));
	}

	addSavedVersions(versions: string[]) {
		const savedVersions = this.getSavedVersions();

		for (const v of versions) {
			savedVersions.add(v);
		}

		window.localStorage.setItem('changelogVersions', JSON.stringify(Array.from(savedVersions)));
	}

	async getVersions() {
		return await this.http.get<string[]>(this.targetEndpoint + 'versions', {
			responseType: 'json'
		}).toPromise();
	}

	private versionLoadingPromises = {};
	private versionTexts = {};

	async getVersion(version: string) {
		if (this.versionTexts[version]) {
			return this.versionTexts[version];
		}

		if (this.versionLoadingPromises[version]) {
			return this.versionLoadingPromises[version];
		}

		this.versionLoadInProgress.next(true);
		this.loadsInProgress++;

		this.versionLoadingPromises[version] = this.http.get(this.targetEndpoint + 'versions/' + version, {
			responseType: 'text'
		}).toPromise();

		const result = this.versionTexts[version] = await this.versionLoadingPromises[version];

		this.versionLoadingPromises[version] = null;

		this.loadsInProgress--;

		if (this.loadsInProgress === 0) {
			this.versionLoadInProgress.next(false);
		}

		return result;
	}
}
