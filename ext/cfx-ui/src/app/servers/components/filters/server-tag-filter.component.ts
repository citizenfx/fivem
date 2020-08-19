import { Component, ChangeDetectorRef } from '@angular/core';
import { ServerTags } from './server-filter-container';
import { ServerTagsService, ServerTag, ServerLocale } from '../../server-tags.service';
import { GameService } from 'app/game.service';
import { FiltersService } from '../../filters.service';

@Component({
	moduleId: module.id,
	selector: 'app-server-tag-filter',
	templateUrl: 'server-tag-filter.component.html',
	styleUrls: ['server-tag-filter.component.scss']
})
export class ServerTagFilterComponent {
	filtersTags: ServerTags = new ServerTags();

	gameName = 'gta5';

	get tags() {
		return this.tagService.tags;
	}

	get locales() {
		return this.tagService.locales;
	}

	get tagsCanReset() {
		return Object.keys(this.filtersTags.tagList).length > 0;
	}

	get localesCanReset() {
		return Object.keys(this.filtersTags.localeList).length > 0;
	}

	constructor(
		private tagService: ServerTagsService,
		private gameService: GameService,
		private cdr: ChangeDetectorRef,
		private filtersService: FiltersService,
	) {
		this.gameName = gameService.gameName;
		this.filtersTags = filtersService.tags;

		tagService.onUpdate.subscribe(() => {
			this.cdr.markForCheck();

			if (this.filtersTags?.tagList) {
				for (const [filterKey] of Object.entries(this.filtersTags.tagList)) {
					if (!this.tagService.tags.some(e => e.name === filterKey)) {
						this.tagService.tags.push({
							name: filterKey,
							count: 0
						});
					}
				}
			}

			if (this.filtersTags?.localeList) {
				for (const [filterKey] of Object.entries(this.filtersTags.localeList)) {
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

		filtersService.tagsUpdates.subscribe((tags) => {
			this.filtersTags = tags;

			this.cdr.markForCheck();
		});
	}

	tagName(tag: ServerTag) {
		return tag?.name;
	}

	localeName(tag: ServerLocale) {
		return tag?.name;
	}

	isLocaleActive(locale: ServerLocale) {
		return (locale?.name in this.filtersTags.localeList) && (this.filtersTags.localeList[locale?.name]);
	}

	isLocaleInactive(locale: ServerLocale) {
		return (locale?.name in this.filtersTags.localeList) && (!this.filtersTags.localeList[locale?.name]);
	}

	isActive(tag: ServerTag) {
		return (tag?.name in this.filtersTags.tagList) && (this.filtersTags.tagList[tag?.name]);
	}

	isInactive(tag: ServerTag) {
		return (tag?.name in this.filtersTags.tagList) && (!this.filtersTags.tagList[tag?.name]);
	}

	toggleTag(tag: ServerTag) {
		if (!(tag.name in this.filtersTags.tagList)) {
			this.filtersTags.tagList[tag.name] = true;
		} else if (this.filtersTags.tagList[tag.name]) {
			this.filtersTags.tagList[tag.name] = false;
		} else {
			delete this.filtersTags.tagList[tag.name];
		}

		this.emitTagsChanged();
	}

	toggleLocale(tag: ServerLocale) {
		if (!(tag.name in this.filtersTags.localeList)) {
			this.filtersTags.localeList[tag.name] = true;
		} else if (this.filtersTags.localeList[tag.name]) {
			this.filtersTags.localeList[tag.name] = false;
		} else {
			delete this.filtersTags.localeList[tag.name];
		}

		this.emitTagsChanged();
	}

	resetLocales() {
		this.filtersTags.localeList = {};
		this.emitTagsChanged();
	}

	resetTags() {
		this.filtersTags.tagList = {};
		this.emitTagsChanged();
	}

	private emitTagsChanged() {
		this.filtersService.setTags(this.filtersTags);
	}
}
