import { Injectable } from '@angular/core';
import { Http, ResponseContentType } from '@angular/http';
import { DomSanitizer } from '@angular/platform-browser';

import 'rxjs/add/operator/toPromise';

import { Server, ServerIcon } from './server';

import { master } from './master';

import idb from 'idb';

@Injectable()
export class ServersService {
    constructor(private http: Http, private domSanitizer: DomSanitizer) { }

    public getServers(): Promise<Server[]> {
        return this.http.get('https://runtime.fivem.net/servers/proto', { responseType: ResponseContentType.ArrayBuffer })
            .toPromise()
            .then(result =>
                master.Servers.decode(new Uint8Array(result.arrayBuffer())).servers.
                    filter(a => a.Data).
                    map((value) => Server.fromObject(this.domSanitizer, value.EndPoint, value.Data)));
    }

    public async getIcons(servers: Server[]): Promise<ServerIcon[]> {
        return servers
            .filter(a => a.int.iconVersion)
            .map(a => new ServerIcon(
                a.address,
                `https://runtime.fivem.net/servers/icon/${a.address}/${a.int.iconVersion}.png`,
                a.int.iconVersion));
    }
}
