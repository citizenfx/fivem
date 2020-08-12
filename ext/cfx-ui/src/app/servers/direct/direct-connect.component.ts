import { Component, OnInit, AfterViewInit, ViewChild, ViewChildren, Inject } from '@angular/core';
import { Subject } from 'rxjs/Subject';

import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/distinctUntilChanged';

import { Server, ServerHistoryEntry } from '../server';

import { ServersService } from '../servers.service';

import { GameService } from '../../game.service';

import { DomSanitizer } from '@angular/platform-browser';
import { L10N_LOCALE, L10nLocale } from 'angular-l10n';
import { DirectConnectBackendComponent } from './direct-connect-backend.component';

class ServerHistoryData {
    entry: ServerHistoryEntry;
    server: Server;
    sanitizedIcon: any;
}

@Component({
    moduleId: module.id,
    selector: 'app-direct-connect',
    templateUrl: 'direct-connect.component.html',
    styleUrls: ['direct-connect.component.scss']
})

export class DirectConnectComponent implements OnInit, AfterViewInit {
    addr = '';
    lastAddr = '';
    server: Server;
    error = '';
    inputInvalid = false;

    @ViewChild('backend')
    backend: DirectConnectBackendComponent;

    onFetchCB: () => void;

    @ViewChildren('input')
    private inputBox;

    history: ServerHistoryData[];

    constructor(private gameService: GameService, private serversService: ServersService, private sanitizer: DomSanitizer,
        @Inject(L10N_LOCALE) public locale: L10nLocale) {
        this.history = this.gameService.getServerHistory().slice(-6).map(entry => ({
            entry,
            server: null,
            sanitizedIcon: (entry.icon ? this.sanitizer.bypassSecurityTrustUrl(entry.icon) : null)
        })).reverse();

        this.serversService
            .getReplayedServers()
            .filter(server => server != null)
            .map(server => ({ server, history: this.history.find(history => history.entry.address === server.address) }))
            .filter(bundle => bundle.history !== undefined)
            .subscribe(bundle => {
                bundle.history.entry.hostname = bundle.server.hostname;
                bundle.history.server = bundle.server;
                bundle.history.sanitizedIcon = bundle.server.sanitizedUri;
            });

        if (this.history.length > 0) {
            const entry = this.history.slice(0, 1)[0].entry;

            this.addrChanged(entry.title || entry.address);
        }
    }

    tryConnect() {
        if (this.backend.isValid()) {
            this.backend.attemptConnect();
        } else {
            const addr = this.backend.addr;

            this.onFetchCB = () => {
                if (addr === this.backend.addr) {
                    this.backend.attemptConnect()
                }
            };
        }
    }

    attemptConnectTo(entry: ServerHistoryData) {
        if (!entry.server) {
            this.gameService.queryAddress(this.parseAddress(entry.entry.title || entry.entry.address))
                .then(server => {
                    this.gameService.connectTo(server, entry.entry.title || entry.entry.address);
                }, (reason: Error) => this.error = reason.message);
        } else {
            this.gameService.connectTo(entry.server, entry.entry.title);
        }
    }

    parseAddress(addr: string): [string, number] {
		if (!addr) {
			 return null;
		}

        const addrBits: [string, number] = [ '', 30120 ];
        const match = addr.match(/^(?:((?:[^\[: ]+)|\[(?:[a-f0-9:]+)\])(?::([0-9]+)|$)|cfx\.re\/join\/[0-9a-z]+)/i);

        if (!match) {
            return null;
        }

        addrBits[0] = match[1];

        if (match[2]) {
            addrBits[1] = parseInt(match[2], 10);
        }

        return addrBits;
    }

    addrChanged(newValue: string) {
        this.addr = newValue;
    }

    ngOnInit() { }

    ngAfterViewInit() {
        this.inputBox.first.nativeElement.focus();
    }
}
