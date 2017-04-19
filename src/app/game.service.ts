import { Injectable } from '@angular/core';

import { Server } from './servers/server';

export interface IGameService {
    init(): void;

    connectTo(server: Server): void;
}

@Injectable()
export class GameService implements IGameService {
    init() {

    }

    connectTo(server: Server) {
        (<any>window).invokeNative('connectTo', server.address);
    }
}

@Injectable()
export class DummyGameService implements IGameService {
    init() {

    }

    connectTo(server: Server) {
        console.log('faking connection to ' + server.address);
    }
}