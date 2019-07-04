import { Component, OnInit, OnChanges, OnDestroy, Input, Output, EventEmitter } from '@angular/core';
import { ServerTags } from './server-filter.component';
import { ServersService } from '../servers.service';
import { Server } from '../server';

class ServerTag {
    public name: string;
    public count: number;
}

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

    serverTags: {[addr: string]: string[]} = {};

    constructor(private serversService: ServersService) {
        this.serversService
            .getReplayedServers()
            .filter(server => !!server)
            .subscribe(server => {
                this.addFilterIndex(server);
            });

        this.serversService
            .getReplayedServers()
            .bufferTime(500)
            .subscribe(server => {
                this.updateTagList();
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
        if (server && server.data && server.data.vars && server.data.vars.tags) {
            const tags: string[] = (<string>server.data.vars.tags)
                .split(',')
                .map(a => a.trim().toLowerCase())
                .filter(a => a);

            this.serverTags[server.address] = tags;
        }
    }

    tagName(tag: ServerTag) {
        return tag.name;
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

    private emitTagsChanged() {
        this.tagsChanged.emit(this.filters);
        localStorage.setItem(`stags:${this.type}`, JSON.stringify(this.filters));
    }

    ngOnInit(): void {
        if (!this.filters.tagList) {
            this.filters.tagList = {};
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

            this.lastType = this.type;
        }

        this.emitTagsChanged();
    }
}
