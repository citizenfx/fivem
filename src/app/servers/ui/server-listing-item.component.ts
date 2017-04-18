import { Component, Input } from '@angular/core';
import {DomSanitizer} from '@angular/platform-browser';

import { Server } from '../server';

@Component({
    moduleId: module.id,
    selector: 'app-server-listing-item',
    templateUrl: 'server-listing-item.component.html',
    styleUrls: ['server-listing-item.component.scss']
})

export class ServerListingItemComponent {
    @Input()
    server: Server;

    constructor(private sanitizer:DomSanitizer) { }

    sanitize(url:string){
        return this.sanitizer.bypassSecurityTrustUrl(url);
    }
}