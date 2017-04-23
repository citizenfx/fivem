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
}

@Injectable()
export class DummyGameService extends GameService {
    init() {

    }

    connectTo(server: Server) {
        console.log('faking connection to ' + server.address);
    }
}