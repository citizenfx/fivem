import { Injectable, EventEmitter } from '@angular/core';
import { ServersService } from './servers.service';

import { Server } from './server';
import cldrLocales from 'cldr-data/main/en/localeDisplayNames.json';
import cldrLanguages from 'cldr-data/main/en/languages.json';
import cldrTerritories from 'cldr-data/main/en/territories.json';
import cldrSubTags from 'cldr-data/supplemental/likelySubtags.json';
import * as cldrjs from 'cldrjs';
import { getCanonicalLocale } from './components/utils';

export class ServerTag {
    public name: string;
    public count: number;
}

export class ServerLocale {
    public name: string;
    public displayName: string;
    public count: number;
    public countryName: string;
}

cldrjs.load(cldrLocales, cldrLanguages, cldrTerritories, cldrSubTags);

@Injectable()
export class ServerTagsService {
    serverTags: {[addr: string]: string[]} = {};
    serverLocale: {[addr: string]: string} = {};

    tags: ServerTag[] = [];
    locales: ServerLocale[] = [];

    onUpdate = new EventEmitter<void>();

    constructor(private serversService: ServersService) {
        this.serversService
            .getReplayedServers()
            .filter(server => !!server)
            .subscribe(server => {
                this.addFilterIndex(server);
                this.addLocaleIndex(server);
            });

        this.serversService
            .getReplayedServers()
            .bufferTime(500)
            .subscribe(server => {
                this.updateTagList();
                this.updateLocaleList();

                this.onUpdate.emit();
            });
    }

    private updateTagList() {
        const tagList = Object.entries(
            Object.values(this.serverTags)
                 .reduce<{[k: string]: number}>((acc: {[k: string]: number}, val: string[]) => {
                    for (const str of val) {
                        if (!acc.hasOwnProperty(str)) {
                            acc[str] = 0;
                        }

                        acc[str]++;
                    }
                    return acc;
                 }, {})
            )
            .map(([name, count]) => {
                return {
                    name,
                    count
                }
            });

        tagList.sort((a, b) => {
            if (a.count === b.count) {
                return 0;
            } else if (a.count > b.count) {
                return -1;
            } else {
                return 1;
            }
        });

        this.tags = tagList.slice(0, 50);
    }

    private addFilterIndex(server: Server) {
        if (this.serverTags[server.address]) {
            return;
        }

        if (server && server.data && server.data.vars && server.data.vars.tags) {
            const tags: string[] = (<string>server.data.vars.tags)
                .split(',')
                .map(a => a.trim().toLowerCase())
                .filter(a => a);

            this.serverTags[server.address] = tags;
        }
    }

    private updateLocaleList() {
        const localeList = Object.entries(
            Object.values(this.serverLocale)
                 .reduce<{[k: string]: number}>((acc: {[k: string]: number}, val: string) => {
                    if (!acc.hasOwnProperty(val)) {
                        acc[val] = 0;
                    }

                    acc[val]++;

                    return acc;
                 }, {})
            )
            .filter(([name, count]) => name.indexOf('-') > 0 && name.indexOf('root') !== 0)
            .map(([name, count]) => {
                const parts = name.split('-');
                const t = parts[parts.length - 1];

                return {
                    name,
                    displayName: this.getLocaleDisplayName(name),
                    count,
                    countryName: t
                }
            });

        localeList.sort((a, b) => {
            if (a.count === b.count) {
                return 0;
            } else if (a.count > b.count) {
                return -1;
            } else {
                return 1;
            }
        });

        this.locales = localeList;
    }

    public getLocaleDisplayName(name: string): string {
        const c = new cldrjs('en');

        const parts = name.split('-');
        const l = parts[0];
        const t = parts[parts.length - 1];

        const lang = c.main('localeDisplayNames/languages/' + l);
        const territory = c.main('localeDisplayNames/territories/' + t);

        return c.main('localeDisplayNames/localeDisplayPattern/localePattern')
            .replace('{0}', lang)
            .replace('{1}', territory);
    }

    private addLocaleIndex(server: Server) {
        if (this.serverLocale[server.address]) {
            return;
        }

        if (server && server.data && server.data.vars && server.data.vars.locale) {
            this.serverLocale[server.address] = getCanonicalLocale(server.data.vars.locale);
        }
    }
}
