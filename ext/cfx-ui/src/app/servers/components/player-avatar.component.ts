import { Component, Input, OnInit, OnChanges, SimpleChanges, PLATFORM_ID, Inject } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer, SafeStyle, SafeUrl } from '@angular/platform-browser';

import { Avatar } from '../avatar';

import { defer, from, Observable, of } from 'rxjs';
import { delay, share, flatMap, catchError } from 'rxjs/operators';

import { Int64BE } from 'int64-buffer';
import { xml2js, ElementCompact } from 'xml-js';
import { isPlatformBrowser } from '@angular/common';

import { DBSchema, IDBPDatabase, openDB } from 'idb';
import { DiscourseService } from 'app/discourse.service';

interface CachedAvatar {
	url: string;
	until: Date;
}

interface AvatarCacheDB extends DBSchema {
	avatars: {
		value: CachedAvatar;
		key: string;
	}
}

class AvatarCache {
	private inited = false;
	private db: IDBPDatabase<AvatarCacheDB>;

	constructor() {
		if (window && window.localStorage) {
			for (const key of Object.keys(localStorage).filter(a => a.startsWith('avatar:'))) {
				localStorage.removeItem(key);
			}
		}

		this.init();
	}

	async init() {
		this.db = await openDB<AvatarCacheDB>('avatar-cache', 1, {
			upgrade(db) {
				db.createObjectStore('avatars');
			}
		});

		this.inited = true;
	}

	async getAvatar(id: string) {
		if (this.inited) {
			const item = await this.db.get('avatars', `avatar:${id}`);

			if (item && new Date() < new Date(item.until)) {
				return item.url;
			}
		}

		return null;
	}

	async setAvatar(id: string, url: string) {
		if (this.inited) {
			const until = new Date();
			until.setTime(until.getTime() + (2 * 86400 * 1000));

			await this.db.put('avatars', {
				url,
				until
			}, `avatar:${id}`);
		}
	}
}

const avatarCache = new AvatarCache();

@Component({
	moduleId: module.id,
	selector: 'app-player-avatar',
	template: `<figure [style.background-image]="svgImage">
		<img
			loading="lazy"
			*ngIf="avatarUrl"
			[src]="avatarUrl"
		>
	</figure>`,
	styles: ['figure, figure img, figure ::ng-deep img { width: 100%; height: 100%; background-repeat: no-repeat; background-size: cover; }'],
})
export class PlayerAvatarComponent implements OnInit, OnChanges {
	@Input()
	public player: any;

	public svgImage: SafeStyle;

	public avatarUrl: string | SafeUrl;

	private svgUrl: string;

	constructor(private sanitizer: DomSanitizer, private http: HttpClient, @Inject(PLATFORM_ID) private platformId: any) {

	}

	public ngOnInit() {
		const svg = Avatar.getFor(this.player.identifiers[0]);
		this.svgUrl = `data:image/svg+xml,${encodeURIComponent(svg)}`;

		this.svgImage = this.sanitizer.bypassSecurityTrustStyle(`url('${this.svgUrl}')`);

		// delay fetching so that we get a chance to load any activity feed first
		of(null)
			.pipe(
				delay(1500),
				flatMap(() => this.getPlayerAvatar()),
				share()
			)
			.subscribe((url) => this.avatarUrl = url);
	}

	private async getCachedAvatar(id: string) {
		return await avatarCache.getAvatar(id);
	}

	private async cacheAvatar(id: string, url: string) {
		await avatarCache.setAvatar(id, url);
	}

	private getPlayerAvatar(): Observable<string | SafeUrl> {
		return defer(() => from(this.getAvatarRaw()));
	}

	private async getAvatarRaw(): Promise<string | SafeUrl> {
		for (const identifier of this.player.identifiers) {
			if (!this.isBrowser()) {
				continue;
			}

			const stringId = (<string>identifier);

			if (stringId.startsWith('steam:') || stringId.startsWith('fivem:')) {
				const cached = await this.getCachedAvatar(stringId);

				if (cached) {
					return cached;
				}
			}

			if (stringId.startsWith('steam:')) {
				const int = new Int64BE(stringId.substr(6), 16);
				const decId = int.toString(10);

				return await this.http.get(`https://steamcommunity.com/profiles/${decId}?xml=1`, { responseType: 'text' })
					.pipe(catchError(() => of('')))
					.map(a => {
						try {
							if (a && a !== '') {
								const obj = xml2js(a, { compact: true }) as ElementCompact;

								if (obj && obj.profile && obj.profile.avatarMedium) {
									const av = obj.profile.avatarMedium._cdata
										.replace('http://cdn.edgecast.steamstatic.com/', 'https://steamcdn-a.akamaihd.net/');
									this.cacheAvatar(stringId, av);
									return av;
								}
							}
						} catch {}

						return this.sanitizer.bypassSecurityTrustUrl(this.svgUrl);
					})
					.toPromise();
			} else if (stringId.startsWith('fivem:')) {
				return await this.http.get(`https://policy-live.fivem.net/api/getUserInfo/${stringId.substring(6)}`, { responseType: 'json' })
					.pipe(catchError(() => of({})))
					.map((a: any) => {
						if (a?.avatar_template) {
							const av = DiscourseService.getAvatarUrlForUser(a.avatar_template);
							this.cacheAvatar(stringId, av);
							return av;
						}

						return this.sanitizer.bypassSecurityTrustUrl(this.svgUrl);
					})
					.toPromise();
			}
		}

		return this.sanitizer.bypassSecurityTrustUrl(this.svgUrl);
	}

	isBrowser() {
		return isPlatformBrowser(this.platformId);
	}

	public ngOnChanges(changes: SimpleChanges) {

	}
}
