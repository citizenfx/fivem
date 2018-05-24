import { Component, OnInit, OnDestroy, ViewChild, ViewChildren, ChangeDetectorRef } from '@angular/core';
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
    error = '';
    canRefresh = true;

    interval: number;

    currentAddr = '';

    onFetchCB: () => void;

    addrEvent = new Subject<[string, number]>();

    disallowedVars = ['sv_enhancedHostSupport', 'sv_licenseKeyToken', 'sv_lan', 'sv_maxClients'];
    
    @Language()
    lang: string;

    @ViewChildren('input')
    private inputBox;

    constructor(private gameService: GameService, private serversService: ServersService,
        private route: ActivatedRoute, private cdRef: ChangeDetectorRef, private sanitizer: DomSanitizer,
        private router: Router) {
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

        this.route.params.subscribe(params => {
            this.currentAddr = params['addr'];

            this.updateServer();
        });
    }

    private updateServer() {
        this.serversService.getServer(this.currentAddr)
            .then(a => {
                this.server = a;
                this.resourceString = (<string[]>a.data.resources)
                    .filter(res => res !== '_cfx_internal' && res !== 'hardcap' && res !== 'sessionmanager')
                    .join(', ');

                this.serverVariables = Object.entries(a.data.vars)
                    .map(([key, value]) => ( { key, value }) )
                    .filter(({ key }) => this.disallowedVars.indexOf(key) < 0)
                    .filter(({ key }) => key.indexOf('banner_') < 0)
                    .map(pair => this.filterFuncs[pair.key] ? this.filterFuncs[pair.key](pair) : pair);
            });
    }

    refreshServer() {
        if (this.canRefresh) {
            this.updateServer();

            this.canRefresh = false;
            window.setTimeout(() => {
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
        this.interval = window.setInterval(() => this.updateServer(), 45000);
    }

    ngOnDestroy() {
        if (this.interval) {
            window.clearInterval(this.interval);
        }
    }
}
