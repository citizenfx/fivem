import { Component, OnInit, OnChanges, OnDestroy, Input, Output, EventEmitter } from '@angular/core';
import { ServerTags } from './server-filter.component';
import { ServersService } from '../servers.service';
import { Server } from '../server';
import cldrLocales from 'cldr-data/main/en/localeDisplayNames.json';
import cldrLanguages from 'cldr-data/main/en/languages.json';
import cldrTerritories from 'cldr-data/main/en/territories.json';
import cldrSubTags from 'cldr-data/supplemental/likelySubtags.json';
import * as cldrjs from 'cldrjs';
import { getCanonicalLocale } from './utils';

class ServerTag {
    public name: string;
    public count: number;
}

class ServerLocale {
    public name: string;
    public displayName: string;
    public count: number;
    public countryName: string;
}

console.log(cldrLocales, cldrLanguages, cldrTerritories);
cldrjs.load(cldrLocales, cldrLanguages, cldrTerritories, cldrSubTags);

@Component({
    moduleId: module.id,
    selector: 'app-server-tag-filter',
    templateUrl: 'server-tag-filter.component.html',
    styleUrls: ['server-tag-filter.component.scss']
})
export class ServerTagFilterComponent implements OnInit, OnChanges, OnDestroy {
    filters: ServerTags = new ServerTags();

    @Input()
    type: string;

    lastType = '';

    @Output()
    public tagsChanged = new EventEmitter<ServerTags>();

    tags: ServerTag[] = [];
    locales: ServerLocale[] = [];

    serverTags: {[addr: string]: string[]} = {};
    serverLocale: {[addr: string]: string} = {};

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

        if (this.filters && this.filters.tagList) {
            for (const [ filterKey ] of Object.entries(this.filters.tagList)) {
                if (!this.tags.some(e => e.name === filterKey)) {
                    this.tags.push({
                        name: filterKey,
                        count: 0
                    });
                }
            }
        }
    }

    private addFilterIndex(server: Server) {
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
                    displayName: getLocaleDisplayName(name),
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

        if (this.filters && this.filters.localeList) {
            for (const [ filterKey ] of Object.entries(this.filters.localeList)) {
                if (!this.locales.some(e => e.name === filterKey)) {
                    const parts = filterKey.split('-');
                    const t = parts[parts.length - 1];

                    this.locales.push({
                        name: filterKey,
                        displayName: getLocaleDisplayName(filterKey),
                        countryName: t,
                        count: 0
                    });
                }
            }
        }

        function getLocaleDisplayName(name: string): string {
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
    }

    private addLocaleIndex(server: Server) {
        if (server && server.data && server.data.vars && server.data.vars.locale) {
            this.serverLocale[server.address] = getCanonicalLocale(server.data.vars.locale);
        }
    }

    tagName(tag: ServerTag) {
        return tag.name;
    }

    localeName(tag: ServerLocale) {
        return tag.name;
    }

    isLocaleActive(locale: ServerLocale) {
        return (locale.name in this.filters.localeList) && (this.filters.localeList[locale.name]);
    }

    isLocaleInactive(locale: ServerLocale) {
        return (locale.name in this.filters.localeList) && (!this.filters.localeList[locale.name]);
    }

    isActive(tag: ServerTag) {
        return (tag.name in this.filters.tagList) && (this.filters.tagList[tag.name]);
    }

    isInactive(tag: ServerTag) {
        return (tag.name in this.filters.tagList) && (!this.filters.tagList[tag.name]);
    }

    toggleTag(tag: ServerTag) {
        if (!(tag.name in this.filters.tagList)) {
            this.filters.tagList[tag.name] = true;
        } else if (this.filters.tagList[tag.name]) {
            this.filters.tagList[tag.name] = false;
        } else {
            delete this.filters.tagList[tag.name];
        }

        this.emitTagsChanged();
    }

    toggleLocale(tag: ServerLocale) {
        if (!(tag.name in this.filters.localeList)) {
            this.filters.localeList[tag.name] = true;
        } else if (this.filters.localeList[tag.name]) {
            this.filters.localeList[tag.name] = false;
        } else {
            delete this.filters.localeList[tag.name];
        }

        this.emitTagsChanged();
    }

    private emitTagsChanged() {
        this.tagsChanged.emit(this.filters);
        localStorage.setItem(`stags:${this.type}`, JSON.stringify(this.filters));
    }

    ngOnInit(): void {
        if (!this.filters.tagList) {
            this.filters.tagList = {};
        }

        if (!this.filters.localeList) {
            this.filters.localeList = {};
        }
    }

    ngOnDestroy(): void {

    }

    ngOnChanges(): void {
        if (this.type !== this.lastType) {
            const storedFilters = localStorage.getItem(`stags:${this.type}`);

            if (storedFilters) {
                this.filters = {...<ServerTags>JSON.parse(storedFilters)};
            } else {
                this.filters = new ServerTags();
            }

            if (!this.filters.tagList) {
                this.filters.tagList = {};
            }

            if (!this.filters.localeList) {
                this.filters.localeList = {};
            }

            this.lastType = this.type;
        }

        this.emitTagsChanged();
    }
}
