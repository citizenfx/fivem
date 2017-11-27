import {DomSanitizer} from '@angular/platform-browser';
import { EventEmitter } from '@angular/core';

import { master } from './master';

import { Avatar } from './avatar';

export class Server {
    readonly address: string;
    readonly hostname: string;
    readonly sortname: string;
    readonly strippedname: string;
    readonly maxPlayers: number;
    readonly data: any;
    readonly int: master.IServerData;

    onChanged = new EventEmitter<void>();

    realIconUri: string;

    get iconUri(): string {
        return this.realIconUri;
    }

    set iconUri(value: string) {
        this.realIconUri = value;
        this.sanitizedUri = this.sanitizer.bypassSecurityTrustUrl(value);
        this.sanitizedStyleUri = this.sanitizer.bypassSecurityTrustStyle('url(' + value + ')');
    }

    sanitizedUri: any;
    sanitizedStyleUri: any;
    currentPlayers: number;
    ping = 9999;

    public updatePing(newValue: number): void {
        this.ping = newValue;

        this.onChanged.emit();
    }

    public getSortable(name: string): any {
        switch (name) {
            case 'name':
                return this.sortname;
            case 'ping':
                return this.ping;
            case 'players':
                return this.currentPlayers;
            default:
                throw new Error('Unknown sortable');
        }
    }

    public static fromObject(sanitizer: DomSanitizer, address: string, object: master.IServerData): Server {
        return new Server(sanitizer, address, object);
    }

    public static fromNative(sanitizer: DomSanitizer, object: any): Server {
        const mappedData = {
            hostname: object.name,
            clients: object.clients,
            svMaxclients: object.maxclients,
            resources: [],
            mapname: object.mapname,
            gametype: object.gametype
        };

        const server = new Server(sanitizer, object.addr, { ...mappedData });

        if (object.infoBlob) {
            server.iconUri = 'data:image/png;base64,' + object.infoBlob.icon;
        }

        return server;
    }

    private constructor(private sanitizer: DomSanitizer, address: string, object: master.IServerData) {
        // temp compat behavior
        this.address = address;
        this.hostname = object.hostname;
        this.currentPlayers = object.clients | 0;
        this.maxPlayers = object.svMaxclients | 0;

        this.strippedname = (this.hostname || '').replace(/\^[0-9]/g, '').normalize('NFD').replace(/[\u0300-\u036f]/g, '');
        this.sortname = this.strippedname.replace(/[^a-zA-Z0-9]/g, '').toLowerCase();

        this.data = object;
        this.int = object;

        if (!object.iconVersion) {
            const svg = Avatar.getFor(this.address);

            this.iconUri = `data:image/svg+xml,${encodeURIComponent(svg)}`;
        } else {
            this.iconUri = `https://runtime.fivem.net/servers/icon/${address}/${object.iconVersion}.png`;
        }
    }
}

export class ServerIcon {
    readonly addr: string;
    readonly icon: string;
    readonly iconVersion: number;

    public constructor(addr: string, icon: string, iconVersion: number) {
        this.addr = addr;
        this.icon = icon;
        this.iconVersion = iconVersion;
    }
}

export class PinConfig {
    pinIfEmpty = false;

    pinnedServers: string[] = [];
}