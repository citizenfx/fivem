import { Component, OnInit, AfterViewInit, ViewChild, ViewChildren } from '@angular/core';
import { Subject } from 'rxjs/Subject';

import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/distinctUntilChanged';

import { Server } from '../server';

import { ServersService } from '../servers.service';

import { GameService, ServerHistoryEntry } from '../../game.service';

import { DomSanitizer } from '@angular/platform-browser';
import { Language } from 'angular-l10n';

class ServerHistoryData {
    entry: ServerHistoryEntry;
    server: Server;
    sanitizedIcon: any;
}

@Component({
    moduleId: module.id,
    selector: 'app-direct-connect',
    templateUrl: 'direct-connect.component.html',
    styleUrls: ['direct-connect.components.scss']
})

export class DirectConnectComponent implements OnInit, AfterViewInit {
    addr = '';
    lastAddr = '';
    server: Server;
    error = '';
    inputInvalid = false;

    @Language()
    lang: string;

    onFetchCB: () => void;

    addrEvent = new Subject<[string, number]>();

    @ViewChildren('input')
    private inputBox;

    history: ServerHistoryData[];

    constructor(private gameService: GameService, private serversService: ServersService, private sanitizer: DomSanitizer) {
        this.addrEvent
            .asObservable()
            .debounceTime(750)
            .distinctUntilChanged()
            .subscribe(addr => {
                this.server = null;
                this.error = null;

                this.gameService.queryAddress(addr)
                    .then(server => {
                        this.server = server;

                        if (this.onFetchCB) {
                            this.onFetchCB();
                        }
                    }, (reason: Error) => this.error = reason.message)
                    .then(() => this.lastAddr = this.addr)
                    .then(() => this.onFetchCB = null);
            });

        this.history = this.gameService.getServerHistory().slice(-6).map(entry => ({
            entry,
            server: null,
            sanitizedIcon: (entry.icon ? this.sanitizer.bypassSecurityTrustUrl(entry.icon) : null)
        })).reverse();

        this.serversService
            .getReplayedServers()
            .filter(server => server != null)
            .map(server => ({ server, history: this.history.find(history => history.entry.address === server.address
                || (server.data && server.data.vars && history.entry.token === server.data.vars.sv_licenseKeyToken)) }))
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
        if (this.isValid()) {
            this.attemptConnect();
        } else {
            const addr = this.addr;

            this.onFetchCB = () => {
                if (addr === this.addr) {
                    this.attemptConnect()
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
        this.inputInvalid = false;
        this.addr = newValue;

        const addrBits = this.parseAddress(newValue);

        if (!addrBits) {
            this.inputInvalid = true;
            return;
        }

        this.addrEvent.next(addrBits);
    }

    attemptConnect() {
        this.gameService.connectTo(this.server, this.addr);
    }

    ngOnInit() { }

    ngAfterViewInit() {
        this.inputBox.first.nativeElement.focus();
    }

    isWaiting() {
        return (this.addr.trim() !== '' && ((!this.server && !this.error) || this.lastAddr !== this.addr));
    }

    isInvalid() {
        return (this.error && this.lastAddr === this.addr);
    }

    isValid() {
        return (this.server && !this.error && this.lastAddr === this.addr);
    }
}
