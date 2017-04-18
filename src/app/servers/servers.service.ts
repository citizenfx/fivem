import { Injectable } from '@angular/core';
import { Http, ResponseContentType } from '@angular/http';

import 'rxjs/add/operator/toPromise';

import { Server, ServerIcon } from './server';

import { master } from './master';

import idb from 'idb';

@Injectable()
export class ServersService {
    constructor(private http: Http) { }

    public getServers(): Promise<Server[]> {
        return this.http.get('https://runtime.fivem.net/servers/proto', { responseType: ResponseContentType.ArrayBuffer })
            .toPromise()
            .then(result =>
                master.Servers.decode(new Uint8Array(result.arrayBuffer())).servers.
                    filter(a => a.Data).
                    map((value) => Server.fromObject(value.EndPoint, value.Data)));
    }

    public async getIcons(servers: Server[]): Promise<ServerIcon[]> {
        const iconDatabase = await idb.open('servers', 1, upgradeDB => {
            switch (upgradeDB.oldVersion) {
                case 0:
                    upgradeDB.createObjectStore('icons');
            }
        });

        const txn = iconDatabase.transaction('icons', 'readonly');
        const iconStore = txn.objectStore('icons');

        const iconsWait = servers.map(a => [a.address, iconStore.get(`serverIcon:${a.address}:${a.int.iconVersion}`)]);
        const icons = 
            (await Promise.all(iconsWait.map(a => a[1])))
                .map((icon, idx) => new ServerIcon(servers[idx].address, icon, servers[idx].int.iconVersion));

        const existingIcons = icons.filter(a => a.icon != null);

        const result = await this.http.post('https://runtime.fivem.net/servers/icons',
            icons.filter(a => a.icon == null).map(a => a.addr),
            {
                responseType: ResponseContentType.ArrayBuffer
            })
            .toPromise();

        const data = master.ServerIcons.decode(new Uint8Array(result.arrayBuffer())).icons;
        const promises: Promise<ServerIcon>[] = [];

        for (const icon of data) {
            promises.push(new Promise((resolve, reject) => {
                var fr = new FileReader();  
                fr.onload = (ev) =>
                {
                    resolve(new ServerIcon(icon.endPoint, fr.result, icon.iconVersion));
                };

                fr.readAsDataURL(new Blob([icon.icon], { type: 'image/png' }));
            }));
        }

        const v = await Promise.all(promises);

        for (const icon of v) {
            const txn = iconDatabase.transaction('icons', 'readwrite');
            const iconStore = txn.objectStore('icons');
            
            iconStore.put(icon.icon, `serverIcon:${icon.addr}:${icon.iconVersion}`);
        }

        return existingIcons.concat(v);
    }
}
