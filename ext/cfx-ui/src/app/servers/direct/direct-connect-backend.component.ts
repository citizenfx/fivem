import { Component, OnInit, AfterViewInit, ViewChild, ViewChildren, Inject, Output, EventEmitter, Input, OnChanges, ChangeDetectorRef } from '@angular/core';
import { Subject } from 'rxjs/Subject';

import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/distinctUntilChanged';

import { Server } from '../server';

import { ServersService } from '../servers.service';

import { GameService, ServerHistoryEntry } from '../../game.service';

import { DomSanitizer } from '@angular/platform-browser';
import { L10N_LOCALE, L10nLocale } from 'angular-l10n';

@Component({
    moduleId: module.id,
    selector: 'app-direct-connect-backend',
    templateUrl: 'direct-connect-backend.component.html',
    styleUrls: ['direct-connect-backend.component.scss']
})

export class DirectConnectBackendComponent implements OnInit, OnChanges {
    @Input()
    addr = '';
    lastAddr = '';
    server: Server;
    error = '';

    @Input()
    silent = false;

    @Output()
    validate = new EventEmitter();

    @Output()
    invalidate = new EventEmitter();

    @Output()
    onChange = new EventEmitter<string>();

    onFetchCB: () => void;

    addrEvent = new Subject<[string, number]>();

    constructor(private gameService: GameService, private cdr: ChangeDetectorRef,
        @Inject(L10N_LOCALE) public locale: L10nLocale) {
        this.addrEvent
            .asObservable()
            .debounceTime(250)
            .distinctUntilChanged()
            .subscribe(addr => {
                this.server = null;
                this.error = null;

                this.gameService.queryAddress(addr)
                    .then(server => {
                        this.server = server;
                        this.cdr.markForCheck();

                        if (this.onFetchCB) {
                            this.onFetchCB();
                        }
                    }, (reason: Error) => this.error = reason.message)
                    .then(() => this.lastAddr = this.addr)
                    .then(() => this.onFetchCB = null);
            });
    }

    ngOnChanges() {
        if (this.addr !== this.lastAddr) {
            this.addrChanged(this.addr);
            this.lastAddr = this.addr;
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
        this.validate.emit();
        this.addr = newValue;

        const addrBits = this.parseAddress(newValue);

        if (!addrBits) {
            this.invalidate.emit();
            return;
        }

        this.addrEvent.next(addrBits);
        this.onChange.emit(this.addr);
    }

    attemptConnect() {
        this.gameService.connectTo(this.server, this.addr);
    }

    ngOnInit() { }

    isWaiting() {
        return (!this.silent && this.addr.trim() !== '' && ((!this.server && !this.error) || this.lastAddr !== this.addr));
    }

    isInvalid() {
        return (!this.silent && this.error && this.lastAddr === this.addr);
    }

    isValid() {
        return (this.server && !this.error && this.lastAddr === this.addr);
    }
}
