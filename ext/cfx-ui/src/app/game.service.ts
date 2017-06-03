import { Injectable, EventEmitter, NgZone } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';

import { Server } from './servers/server';

export class ConnectStatus {
    public server: Server;
    public message: string;
    public count: number;
    public total: number;
}

@Injectable()
export abstract class GameService {
    connectFailed = new EventEmitter<[Server, string]>();
    connectStatus = new EventEmitter<ConnectStatus>();
    connecting = new EventEmitter<Server>();

    get nickname(): string {
        return 'UnknownPlayer';
    }

    set nickname(name: string) {
        
    }

    abstract init(): void;

    abstract connectTo(server: Server): void;

    abstract pingServers(servers: Server[]): Server[];

    abstract isMatchingServer(type: string, server: Server): boolean;

    abstract toggleListEntry(type: string, server: Server, isInList: boolean): void;

    queryAddress(address: [string, number]): Promise<Server> {
        return new Promise<Server>((resolve, reject) => setTimeout(() => reject(new Error("Querying isn't supported in GameService.")), 2500));
    }

    exitGame(): void {

    }

    cancelNativeConnect(): void {

    }

    protected invokeConnectFailed(server: Server, message: string) {
        this.connectFailed.emit([server, message]);
    }

    protected invokeConnecting(server: Server) {
        this.connecting.emit(server);   
    }

    protected invokeConnectStatus(server: Server, message: string, count: number, total: number) {
        this.connectStatus.emit({
            server: server,
            message: message,
            count: count,
            total: total
        });
    }
}

@Injectable()
export class CfxGameService extends GameService {
    private lastServer: Server;

    private pingList: {[addr: string]: Server} = {};

    private pingListEvents: [string, number][] = [];

    private favorites: string[] = [];

    private history: string[] = [];

    private realNickname: string;

    private inConnecting = false;

    constructor(private sanitizer: DomSanitizer, private zone: NgZone) {
        super();
    }

    init() {
        (<any>window).invokeNative('getFavorites', '');

        this.zone.runOutsideAngular(() => {
            window.addEventListener('message', (event) => {
                switch (event.data.type) {
                    case 'connectFailed':
                        this.zone.run(() => this.invokeConnectFailed(this.lastServer, event.data.message));
                        break;
                    case 'connecting':
                        this.zone.run(() => this.invokeConnecting(this.lastServer));
                        break;
                    case 'connectStatus':
                        this.zone.run(() =>
                            this.invokeConnectStatus(
                                this.lastServer, event.data.data.message, event.data.data.count, event.data.data.total));
                        break;
                    case 'serverAdd':
                        if (event.data.addr in this.pingList) {
                            this.pingListEvents.push([event.data.addr, event.data.ping]);
                        }
                        break;
                    case 'getFavorites':
                        this.zone.run(() => this.favorites = event.data.list);
                        break;
                    case 'addToHistory':
                        this.history.push(event.data.address);
                        this.saveHistory();
                        break;
                }
            });

            window.setInterval(() => {
                if (this.pingListEvents.length > 0) {
                    this.zone.run(() => {
                        const ple = this.pingListEvents;

                        for (const event of ple) {
                            this.pingList[event[0]].updatePing(event[1]);
                        }
                    });
                }

                this.pingListEvents = [];
            }, 500);
        });

        this.history = JSON.parse(localStorage.getItem('history')) || [];

        if (localStorage.getItem('nickOverride')) {
            (<any>window).invokeNative('checkNickname', localStorage.getItem('nickOverride'));
            this.realNickname = localStorage.getItem('nickOverride');
        }

        this.connecting.subscribe(server => {
            this.inConnecting = false;
        })
    }

    get nickname(): string {
        return this.realNickname;
    }

    set nickname(name: string) {
        this.realNickname = name;
        localStorage.setItem('nickOverride', name);

        (<any>window).invokeNative('checkNickname', name);
    }

    private saveHistory() {
        localStorage.setItem('history', JSON.stringify(this.history));
    }

    connectTo(server: Server) {
        if (this.inConnecting) {
            return;
        }

        this.inConnecting = true;

        this.lastServer = server;

        (<any>window).invokeNative('connectTo', server.address);

        // temporary, we hope
        this.history.push(server.address);
        this.saveHistory();
    }

    pingServers(servers: Server[]) {
        for (const server of servers) {
            this.pingList[server.address] = server;
        }

        (<any>window).invokeNative('pingServers', JSON.stringify(
            servers.map(a => [ a.address.split(':')[0], parseInt(a.address.split(':')[1]) ])
        ));

        return servers;
    }

    isMatchingServer(type: string, server: Server) {
        if (type == 'favorites') {
            return this.favorites.indexOf(server.address) >= 0;
        } else if (type == 'history') {
            return this.history.indexOf(server.address) >= 0;
        }

        return true;
    }

    toggleListEntry(list: string, server: Server, isInList: boolean) {
        if (this.isMatchingServer(list, server) !== isInList) {
            if (isInList) {
                if (list == 'favorites') {
                    this.favorites.push(server.address);
                } else if (list == 'history') {
                    this.history.push(server.address);

                }
            } else {
                if (list == 'favorites') {
                    this.favorites = this.favorites.filter(a => a != server.address);
                } else if (list == 'history') {
                    this.history = this.history.filter(a => a != server.address);
                }
            }
        }

        if (list == 'favorites') {
            (<any>window).invokeNative('saveFavorites', JSON.stringify(this.favorites))
        } else if (list == 'history') {
            this.saveHistory();
        }
    }

    cancelNativeConnect(): void {
        (<any>window).invokeNative('cancelDefer', '');
    }

    lastQuery: string;

    queryAddress(address: [string, number]): Promise<Server> {
        const addrString = address[0] + ':' + address[1];

        const promise = new Promise<Server>((resolve, reject) =>
        {
            const to = window.setTimeout(() => {
                if (addrString != this.lastQuery) {
                    return;
                }

                reject(new Error("Server query timed out."));

                window.removeEventListener('message', cb);
            }, 2500);

            const cb = (event) =>
            {
                if (addrString != this.lastQuery) {
                    window.removeEventListener('message', cb);
                    return;
                }

                if (event.data.type == 'queryingFailed') {
                    if (event.data.arg == addrString) {
                        reject(new Error("Failed to query server."));
                        window.removeEventListener('message', cb);
                        window.clearTimeout(to);
                    }
                } else if (event.data.type == 'serverQueried') {
                    resolve(Server.fromNative(this.sanitizer, event.data));
                    window.removeEventListener('message', cb);
                    window.clearTimeout(to);
                }
            };

            window.addEventListener('message', cb);
        });

        (<any>window).invokeNative('queryServer', addrString);
        this.lastQuery = addrString;

        return promise;
    }

    exitGame(): void {
        (<any>window).invokeNative('exit', '');
    }
}

@Injectable()
export class DummyGameService extends GameService {
    init() {
        
    }

    connectTo(server: Server) {
        console.log('faking connection to ' + server.address);

        this.invokeConnecting(server);

        setTimeout(() =>
        {
            this.invokeConnectStatus(server, 'hey!', 12, 12)

            setTimeout(() =>
            {
                this.invokeConnectFailed(server, 'Sorry, we\'re closed. :(');
            }, 2000);
        }, 1500);
    }

    pingServers(servers: Server[]): Server[] {
        return servers;
    }

    isMatchingServer(type: string, server: Server): boolean {
        return ((type != 'history' && type != 'favorites') || server.currentPlayers < 12);
    }

    toggleListEntry(list: string, server: Server, isInList: boolean) {
    }
}