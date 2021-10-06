import { Component, Input, OnInit, OnChanges, SimpleChanges, PLATFORM_ID, Inject } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer, SafeStyle } from '@angular/platform-browser';

import { Avatar } from '../avatar';

import { EMPTY, Observable, of } from 'rxjs';
import { delay, share, flatMap, catchError } from 'rxjs/operators';

import { Int64BE } from 'int64-buffer';
import { xml2js, ElementCompact } from 'xml-js';
import { isPlatformBrowser } from '@angular/common';
import { DiscourseService } from 'app/discourse.service';

interface CachedAvatar {
	url: string;
	until: Date;
}

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

	public avatarUrl: string;

	private svgUrl: string;

	constructor(private sanitizer: DomSanitizer, private http: HttpClient, @Inject(PLATFORM_ID) private platformId: any,
		private discourse: DiscourseService) {

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

	private getCachedAvatar(id: string) {
		const item: CachedAvatar = JSON.parse(localStorage.getItem(`avatar:${id}`));

		if (item && new Date() < new Date(item.until)) {
			return item.url;
		}

		return null;
	}

	private cacheAvatar(id: string, url: string) {
		const until = new Date();
		until.setTime(until.getTime() + (2 * 86400 * 1000));

		localStorage.setItem(`avatar:${id}`, JSON.stringify({
			url,
			until
		}));
	}

	private getPlayerAvatar(): Observable<any> {
		for (const identifier of this.player.identifiers) {
			if (!this.isBrowser()) {
				continue;
			}

			const stringId = (<string>identifier);

			if (stringId.startsWith('steam:') || stringId.startsWith('fivem:')) {
				const cached = this.getCachedAvatar(stringId);

				if (cached) {
					return of(cached);
				}
			}

			if (stringId.startsWith('steam:')) {
				const int = new Int64BE(stringId.substr(6), 16);
				const decId = int.toString(10);

				return this.http.get(`https://steamcommunity.com/profiles/${decId}?xml=1`, { responseType: 'text' })
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
					});
			} else if (stringId.startsWith('fivem:')) {
				return this.http.get(`https://policy-live.fivem.net/api/getUserInfo/${stringId.substring(6)}`, { responseType: 'json' })
					.pipe(catchError(() => of({})))
					.map((a: any) => {
						if (a?.avatar_template) {
							const av = DiscourseService.getAvatarUrlForUser(a.avatar_template);
							this.cacheAvatar(stringId, av);
							return av;
						}

						return this.sanitizer.bypassSecurityTrustUrl(this.svgUrl);
					});
			}
		}

		return of(this.sanitizer.bypassSecurityTrustUrl(this.svgUrl));
	}

	isBrowser() {
		return isPlatformBrowser(this.platformId);
	}

	public ngOnChanges(changes: SimpleChanges) {

	}
}
