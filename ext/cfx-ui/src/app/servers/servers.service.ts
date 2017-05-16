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
        // open the icon storage database
        const iconDatabase = await idb.open('servers', 1, upgradeDB => {
            // needs upgrading?
            switch (upgradeDB.oldVersion) {
                case 0:
                    upgradeDB.createObjectStore('icons');
            }
        });

        // create a read-only transaction for the object store
        const txn = iconDatabase.transaction('icons', 'readonly');
        const iconStore = txn.objectStore('icons');

        // get all icons that already exist
        // this gets a list of promises, then awaits the promises and creates ServerIcon objects
        const iconsWait = servers.map(a => [a.address, iconStore.get(`serverIcon:${a.address}:${a.int.iconVersion}`)]);
        const icons = 
            (await Promise.all(iconsWait.map(a => a[1])))
                .map((icon, idx) => new ServerIcon(servers[idx].address, icon, servers[idx].int.iconVersion));

        // filter out any existing icons
        const existingIcons = icons.filter(a => a.icon != null);

        // request the icons from the runtime
        const result = await this.http.post('https://runtime.fivem.net/servers/icons',
            icons.filter(a => a.icon == null).map(a => a.addr),
            {
                responseType: ResponseContentType.ArrayBuffer
            })
            .toPromise();
        
        // parse the protobuf, and make a list of promises for reading the data: URIs
        const data = master.ServerIcons.decode(new Uint8Array(result.arrayBuffer())).icons;
        const promises: Promise<ServerIcon>[] = [];

        // convert the icons to data URIs
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

        // asynchronously (we don't care about the response) cache the icons
        for (const icon of v) {
            // this makes a new transaction each time as the last are probably GC'd by now, so
            // it'll error out otherwise
            const txn = iconDatabase.transaction('icons', 'readwrite');
            const iconStore = txn.objectStore('icons');
            
            iconStore.put(icon.icon, `serverIcon:${icon.addr}:${icon.iconVersion}`);
        }

        // and return the new icons _and_ the existing icons
        return existingIcons.concat(v);
    }
}
