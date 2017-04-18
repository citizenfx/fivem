import { master } from './master';

import { Avatar } from './avatar';

export class Server {
    readonly address: string;
    readonly hostname: string;
    readonly sortname: string;
    readonly strippedname: string;
    readonly data: any;
    readonly int: master.ServerData$Properties;

    iconUri: string;
    currentPlayers: number;

    get maxPlayers(): number {
        return 24;
    }

    get ping(): number {
        return 42;
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

    public static fromObject(address: string, object: master.ServerData$Properties): Server {
        return new Server(address, object);
    }

    private constructor(address: string, object: master.ServerData$Properties) {
        // temp compat behavior
        this.address = address;
        this.hostname = object.hostname;
        this.currentPlayers = object.clients | 0;

        this.strippedname = this.hostname.replace(/\^[0-9]/g, '').normalize('NFD').replace(/[\u0300-\u036f]/g, '');
        this.sortname = this.strippedname.replace(/[^a-zA-Z0-9]/g, '').toLowerCase();

        this.data = object;
        this.int = object;

        const svg = Avatar.getFor(this.address);

        this.iconUri = `data:image/svg+xml,${encodeURIComponent(svg)}`;
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