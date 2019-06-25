import { Component, Input, OnInit, OnChanges, SimpleChanges, PLATFORM_ID, Inject } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer, SafeStyle } from '@angular/platform-browser';

import { Avatar } from '../avatar';

import { Observable } from 'rxjs/Observable';
import { of } from 'rxjs/observable/of';
import { delay, share, map } from 'rxjs/operators';

import { Int64BE } from 'int64-buffer';
import { xml2js, ElementCompact } from 'xml-js';
import { isPlatformBrowser } from '@angular/common';

@Component({
    moduleId: module.id,
    selector: 'app-player-avatar',
    template: `<figure [style.background-image]="svgImage"><img *ngIf="playerUrl | async" [src]="playerUrl | async"></figure>`
})
export class PlayerAvatarComponent implements OnInit, OnChanges {
    @Input()
    public player: any;

    public playerUrl: Observable<any>;

    public svgImage: SafeStyle;

    private svgUrl: string;

    constructor(private sanitizer: DomSanitizer, private http: HttpClient, @Inject(PLATFORM_ID) private platformId: any) {

    }

    public ngOnInit() {
        const svg = Avatar.getFor(this.player.identifiers[0]);
        this.svgUrl = `data:image/svg+xml,${encodeURIComponent(svg)}`;

        this.svgImage = this.sanitizer.bypassSecurityTrustStyle(`url('${this.svgUrl}')`);
        this.playerUrl = this.getPlayerAvatar().pipe(share());
    }

    private getPlayerAvatar(): Observable<any> {
        for (const identifier of this.player.identifiers) {
            const stringId = (<string>identifier);

            if (this.isBrowser() && stringId.startsWith('steam:')) {
                const int = new Int64BE(stringId.substr(6), 16);
                const decId = int.toString(10);

                return this.http.get(`https://steamcommunity.com/profiles/${decId}?xml=1`, { responseType: 'text' })
                                .map(a => {
                                    const obj = xml2js(a, { compact: true }) as ElementCompact;

                                    if (obj && obj.profile && obj.profile.avatarMedium) {
                                        return obj.profile.avatarMedium._cdata
                                            .replace('http://cdn.edgecast.steamstatic.com/', 'https://steamcdn-a.akamaihd.net/');
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
