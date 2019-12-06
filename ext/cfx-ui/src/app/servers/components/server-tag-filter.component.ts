import { Component, OnInit, OnChanges, OnDestroy, Input, Output, EventEmitter } from '@angular/core';
import { ServerTags } from './server-filter-container';
import { ServerTagsService, ServerTag, ServerLocale } from '../server-tags.service';

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

    get tags() {
        return this.tagService.tags;
    }

    get locales() {
        return this.tagService.locales;
    }

    constructor(private tagService: ServerTagsService) {
        tagService.onUpdate.subscribe(() => {
            if (this.filters && this.filters.tagList) {
                for (const [ filterKey ] of Object.entries(this.filters.tagList)) {
                    if (!this.tagService.tags.some(e => e.name === filterKey)) {
                        this.tagService.tags.push({
                            name: filterKey,
                            count: 0
                        });
                    }
                }
            }

            if (this.filters && this.filters.localeList) {
                for (const [ filterKey ] of Object.entries(this.filters.localeList)) {
                    if (!this.locales.some(e => e.name === filterKey)) {
                        const parts = filterKey.split('-');
                        const t = parts[parts.length - 1];
    
                        this.locales.push({
                            name: filterKey,
                            displayName: this.tagService.getLocaleDisplayName(filterKey),
                            countryName: t,
                            count: 0
                        });
                    }
                }
            }    
        });
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
