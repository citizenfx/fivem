import { Injectable, EventEmitter } from '@angular/core';
import { ServersService } from './servers.service';

import { Server } from './server';
import cldrLocales from 'cldr-data/main/en/localeDisplayNames.json';
import cldrLanguages from 'cldr-data/main/en/languages.json';
import cldrTerritories from 'cldr-data/main/en/territories.json';
import cldrSubTags from 'cldr-data/supplemental/likelySubtags.json';
import * as cldrjs from 'cldrjs';
import { getCanonicalLocale } from './components/utils';

export class ServerTag {
	public name: string;
	public count: number;
}

export class ServerLocale {
	public name: string;
	public displayName: string;
	public count: number;
	public countryName: string;
}

cldrjs.load(cldrLocales, cldrLanguages, cldrTerritories, cldrSubTags);

@Injectable()
export class ServerTagsService {
	serverTags: { [addr: string]: true } = {};
	serverLocale: { [addr: string]: true } = {};

	tags: ServerTag[] = [];
	locales: ServerLocale[] = [];

	onUpdate = new EventEmitter<void>();

	private tagsIndex: { [key: string]: number } = {};
	private localesIndex: { [key: string]: number } = {};

	constructor(private serversService: ServersService) {
		this.serversService
			.getReplayedServers()
			.filter((server) => !!server)
			.subscribe(server => {
				this.addServerTags(server);
				this.addLocaleIndex(server);
			});

		this.serversService
			.getReplayedServers()
			.bufferTime(1000)
			.subscribe((servers) => {
				if (servers.length === 0) {
					return;
				}

				this.updateTagList();
				this.updateLocaleList();

				this.onUpdate.emit();
			});
	}

	private updateTagList() {
		const tags = Object.entries(this.tagsIndex).sort((a, b) => b[1] - a[1]);

		tags.length = Math.min(50, tags.length);

		this.tags = tags.map(([name, count]) => ({ name, count }));
	}

	private addServerTags(server: Server) {
		if (this.serverTags[server.address]) {
			return;
		}

		if (server?.data?.vars?.tags) {
			(<string>server.data.vars.tags)
				.split(',')
				.forEach((rawTag) => {
					const tag = rawTag.trim().toLowerCase();

					if (!tag) {
						return;
					}

					this.tagsIndex[tag] = (this.tagsIndex[tag] || 0) + 1;
				});

			this.serverTags[server.address] = true;
		}
	}

	private updateLocaleList() {
		const locales = Object.entries(this.localesIndex).sort((a, b) => b[1] - a[1]);

		this.locales = locales.map(([name, count]) => ({
			name,
			count,
			displayName: this.getLocaleDisplayName(name),
			countryName: name.split('-').reverse()[0],
		}));
	}

	private cachedLocaleDisplayNames: { [key: string]: string } = {};

	public getLocaleDisplayName(name: string): string {
		let cached = this.cachedLocaleDisplayNames[name];

		if (!this.cachedLocaleDisplayNames[name]) {
			const c = new cldrjs('en');

			const parts = name.split('-');
			const l = parts[0];
			const t = parts[parts.length - 1];

			const lang = c.main('localeDisplayNames/languages/' + l);
			const territory = c.main('localeDisplayNames/territories/' + t);

			cached = this.cachedLocaleDisplayNames[name] = c.main('localeDisplayNames/localeDisplayPattern/localePattern')
				.replace('{0}', lang)
				.replace('{1}', territory);
		}

		return cached;
	}

	private addLocaleIndex(server: Server) {
		if (this.serverLocale[server.address]) {
			return;
		}

		if (server?.data?.vars?.locale) {
			this.serverLocale[server.address] = true;

			const locale = getCanonicalLocale(server.data.vars.locale);

			// We don't care about root, right
			if (locale === 'root-AQ') {
				return;
			}

			this.localesIndex[locale] = (this.localesIndex[locale] || 0) + 1;
		}
	}
}
