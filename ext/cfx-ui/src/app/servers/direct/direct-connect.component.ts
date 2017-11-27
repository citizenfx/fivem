import { Component, OnInit, AfterViewInit, ViewChild, ViewChildren } from '@angular/core';
import { Subject } from 'rxjs/Subject';

import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/distinctUntilChanged';

import { Server } from '../server';

import { GameService } from '../../game.service';

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

    onFetchCB: () => void;

    addrEvent = new Subject<[string, number]>();

    @ViewChildren('input')
    private inputBox;

    constructor(private gameService: GameService) {
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

        const lastServer = localStorage.getItem('lastServer');

        if (lastServer) {
            this.addrChanged(lastServer);
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

    addrChanged(newValue: string) {
        this.inputInvalid = false;
        this.addr = newValue;

        const addrBits: [string, number] = [ '', 30120 ];
        const match = newValue.match(/^((?:[^\[: ]+)|\[(?:[a-f0-9:]+)\])(?::([0-9]+)|$)/i);

        if (!match)
        {
            this.inputInvalid = true;
            return;
        }

        addrBits[0] = match[1];

        if (match[2]) {
            addrBits[1] = parseInt(match[2], 10);
        }

        this.addrEvent.next(addrBits);
    }

    attemptConnect() {
        this.gameService.connectTo(this.server);
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
