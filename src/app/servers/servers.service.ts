import { Injectable } from '@angular/core';
import { Http } from '@angular/http';

import 'rxjs/add/operator/toPromise';

import { Server } from './server';

@Injectable()
export class ServersService {
    constructor(private http: Http) { }

    public getServers(): Promise<Server[]> {
        return this.http.get('http://5r.kngrektor.com/api/getservers')
            .toPromise()
            .then(result => Object.entries(result.json().Servers).map(([key, value]) => Server.fromObject(key, value)));
    }
}
