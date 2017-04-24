import { Injectable, EventEmitter } from '@angular/core';

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

    abstract init(): void;

    abstract connectTo(server: Server): void;

    abstract pingServers(servers: Server[]): Server[];

    abstract isMatchingServer(type: string, server: Server): boolean;

    abstract toggleListEntry(type: string, server: Server, isInList: boolean): void;

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

    private favorites: string[] = [];

    private history: string[] = [];

    private inConnecting = false;

    init() {
        (<any>window).invokeNative('getFavorites');

        window.addEventListener('message', (event) => {
            switch (event.data.type) {
                case 'connectFailed':
                    this.invokeConnectFailed(this.lastServer, event.data.message);
                    break;
                case 'connecting':
                    this.invokeConnecting(this.lastServer);
                    break;
                case 'connectStatus':
                    this.invokeConnectStatus(this.lastServer, event.data.message, event.data.count, event.data.total);
                    break;
                case 'serverAdd':
                    if (event.data.addr in this.pingList)
                    {
                        this.pingList[event.data.addr].updatePing(event.data.ping);
                    }
                    break;
                case 'getFavorites':
                    this.favorites = event.data.list;
                    break;
                case 'addToHistory':
                    this.history.push(event.data.address);
                    this.saveHistory();
                    break;
            }
        });

        this.history = JSON.parse(localStorage.getItem('history'));

        this.connecting.subscribe(server => {
            this.inConnecting = false;
        })
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
            (<any>window).invokeNative('saveFavorites', JSON.stringify(list))
        } else if (list == 'history') {
            this.saveHistory();
        }
    }
}

@Injectable()
export class DummyGameService extends GameService {
    init() {
        document.body.style.zoom = ((window.innerHeight / 720) * 100) + '%';
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