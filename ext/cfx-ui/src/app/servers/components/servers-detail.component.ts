import { Component, OnInit, OnDestroy, ViewChild, ViewChildren, ChangeDetectorRef, Inject, PLATFORM_ID } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { DomSanitizer } from '@angular/platform-browser';
import { Subject } from 'rxjs/Subject';

import { Language, Translation, TranslationService } from 'angular-l10n';

import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/distinctUntilChanged';

import { Server } from '../server';
import { Avatar } from '../avatar';

import { GameService } from '../../game.service';

import { ServersService } from '../servers.service';

import { isPlatformBrowser } from '@angular/common';
import { MetaService } from '@ngx-meta/core';

class VariablePair {
    public key: string;
    public value: string;
}

@Component({
    moduleId: module.id,
    selector: 'app-servers-detail',
    templateUrl: 'servers-detail.component.html',
    styleUrls: ['servers-detail.component.scss']
})

export class ServersDetailComponent extends Translation implements OnInit, OnDestroy {
    addr = '';
    lastAddr = '';
    server: Server;
    serverVariables: VariablePair[] = [];
    filterFuncs: {[key: string]: (pair: VariablePair) => VariablePair} = {};
    resourceString = '';
    resourceCount = 0;
    error = '';
    canRefresh = true;

    interval: ReturnType<typeof setInterval>;

    currentAddr = '';

    onFetchCB: () => void;

    addrEvent = new Subject<[string, number]>();

    disallowedVars = ['sv_enhancedHostSupport', 'sv_licenseKeyToken', 'sv_lan', 'sv_maxClients'];

    @Language()
    lang: string;

    @ViewChildren('input')
    private inputBox;

    get serverAddress() {
        if (this.server) {
            if (this.server.address) {
                const address = this.server.address;

                const m = address.match(/(.*)-(.*?)\.cfx\.re/i);

                if (m) {
                    return `cfx.re/join/${m[2]}`;
                }

                const m2 = address.match(/^[a-z0-9]{6,}$/i);

                if (m2) {
                    return `cfx.re/join/${m2[0]}`;
                }

                return address;
            }
        }

        return null;
    }

    constructor(private gameService: GameService, private serversService: ServersService,
        private route: ActivatedRoute, private cdRef: ChangeDetectorRef, private sanitizer: DomSanitizer,
        private router: Router, @Inject(PLATFORM_ID) private platformId: any, private meta: MetaService) {
        super();

        this.filterFuncs['sv_scriptHookAllowed'] = (pair) => {
            return {
                key: '#ServerDetail_ScriptHook',
                value: (pair.value === 'true') ? '#Yes' : '#No'
            };
        };

        this.filterFuncs['onesync_enabled'] = (pair) => {
            return {
                key: '#ServerDetail_OneSync',
                value: (pair.value === 'true') ? '#Yes' : '#No'
            }
        };

        const premiumTiers = {
            ag: 'FiveM Element Club Argentum 💿',
            au: 'FiveM Element Club Aurum 📀',
            pt: 'FiveM Element Club Platinum 🌟'
        };

        this.filterFuncs['premium'] = (pair) => {
            return {
                key: '#ServerDetail_Premium',
                value: (pair.value in premiumTiers) ? premiumTiers[pair.value] : pair.value
            }
        };

        this.filterFuncs['tags'] = (pair) => {
            return {
                key: '#ServerList_Tags',
                value: pair.value
            };
        };

        this.route.params.subscribe(params => {
            this.currentAddr = params['addr'];

            this.updateServer();
        });
    }

    private updateServer() {
        this.serversService.getServer(this.currentAddr)
            .then(a => {
                this.server = a;

                const resources = (<string[]>a.data.resources)
                    .filter(res => res !== '_cfx_internal' && res !== 'hardcap' && res !== 'sessionmanager');

                this.resourceString = resources
                    .join(', ');

                this.resourceCount = resources.length;

                this.serverVariables = Object.entries(a.data.vars as {[key: string]: string })
                    .map(([key, value]) => ( { key, value }) )
                    .filter(({ key }) => this.disallowedVars.indexOf(key) < 0)
                    .filter(({ key }) => key.indexOf('banner_') < 0)
                    .map(pair => this.filterFuncs[pair.key] ? this.filterFuncs[pair.key](pair) : pair);

                this.meta.setTag('og:image', this.server.iconUri);
                this.meta.setTag('og:type', 'website');
                this.meta.setTitle(this.server.hostname.replace(/\^[0-9]/g, ''));
                this.meta.setTag('og:description', `${this.server.currentPlayers} players on ${this.server.data.mapname}`);
                this.meta.setTag('og:site_name', 'FiveM');
            });
    }

    refreshServer() {
        if (this.canRefresh && isPlatformBrowser(this.platformId)) {
            this.updateServer();

            this.canRefresh = false;
            setTimeout(() => {
                this.canRefresh = true;
            }, 2000);
        }
    }

    goBack() {
        this.router.navigate(['/', 'servers']);
    }

    attemptConnect() {
        this.gameService.connectTo(this.server);
    }

    isFavorite() {
        return this.gameService.isMatchingServer('favorites', this.server);
    }

    addFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, true);
    }

    removeFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, false);
    }

    ngOnInit() {
        if (isPlatformBrowser(this.platformId)) {
            this.interval = setInterval(() => this.updateServer(), 45000);
        }
    }

    ngOnDestroy() {
        if (this.interval) {
            clearInterval(this.interval);
        }
    }
}
