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

    private inConnecting = false;

    init() {
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
                    this.pingList[event.data.addr].updatePing(event.data.ping);
                    break;
            }
        });

        this.connecting.subscribe(server => {
            this.inConnecting = false;
        })
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
}