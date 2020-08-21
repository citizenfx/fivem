import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { NgModule, Injectable, Optional, APP_INITIALIZER, Injector } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { HttpClientModule, HttpHeaders, HttpClient } from '@angular/common/http';
import { NgSelectModule } from '@ng-select/ng-select';
import { Nl2BrPipeModule } from 'nl2br-pipe';

import { NgDompurifyModule } from '@tinkoff/ng-dompurify';

import { environment } from '../environments/environment'

import { ScrollingModule } from '@angular/cdk/scrolling';
import { ScrollingModule as ExperimentalScrollingModule } from '@angular/cdk-experimental/scrolling';

import { MomentModule } from 'angular2-moment';
import { Angulartics2Module } from 'angulartics2';
import { LinkyModule } from 'ngx-linky';
import { NgxFilesizeModule } from 'ngx-filesize';
import { MetaModule, MetaLoader, MetaStaticLoader, PageTitlePositioning } from '@ngx-meta/core';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';

import { AppNavComponent } from './nav/app-nav.component';
import { ConnectingPopupComponent } from './servers/connecting-popup.component';
import { HomeComponent } from './home/home.component';
import { HomeTweetComponent } from './home/home-tweet.component';
import { SettingsComponent } from './settings/settings.component';

import { ServersComponent } from './servers/components/servers.component';
import { ServersContainerComponent } from './servers/components/servers-container.component';
import { ServersListComponent } from './servers/components/list/servers-list.component';
import { ServersListItemComponent } from './servers/components/list/servers-list-item.component';
import { ServerFilterComponent } from './servers/components/filters/server-filter.component';
import { ServerTagFilterComponent } from './servers/components/filters/server-tag-filter.component';
import { ServersDetailComponent } from './servers/components/detail/servers-detail.component';
import { PlayerAvatarComponent } from './servers/components/player-avatar.component';
import { DirectConnectComponent } from './servers/direct/direct-connect.component';
import { SpinnerComponent } from './spinner/spinner.component';

import { ServersService } from './servers/servers.service';
import { TweetService } from './home/tweet.service';
import { TrackingService } from './tracking.service';
import { SettingsService } from './settings.service';

import { GameService, CfxGameService, DummyGameService } from './game.service';
import { DiscourseService } from './discourse.service';

import { ColorizePipe } from './colorize.pipe';
import { EscapePipe } from './escape.pipe';
import { LocalStorage } from './local-storage';

import { Languages } from './languages';
import { ServerTagsService } from './servers/server-tags.service';
import { ChangelogPopupComponent } from './changelog/app-changelog-popup.component';
import { ChangelogEntryComponent } from './changelog/app-changelog-entry.component';
import { ChangelogService } from './changelogs.service';
import {
	L10nConfig, L10nTranslationLoader, L10nProvider, L10nLoader, L10nTranslationModule,
	L10nMissingTranslationHandler, L10nTranslationService
} from 'angular-l10n';
import { Observable } from 'rxjs';
import { take } from 'rxjs/operators';

import { MatSelectModule } from '@angular/material/select';
import { MatTabsModule } from '@angular/material/tabs';
import { MatCheckboxModule } from '@angular/material/checkbox';

import { ModsComponent } from './mods/mods/mods.component';
import { ModListComponent } from './mods/mod-list/mod-list.component';
import { ModDetailComponent } from './mods/mod-detail/mod-detail.component';
import { ModsService } from './mods/mods.service';
import { ModsListComponent } from './mods/mods-list/mods-list.component';
import { ModItemComponent } from './mods/mod-item/mod-item.component';
import { CreateHomeComponent } from './create/create-home/create-home.component';
import { StoryHomeComponent } from './story/story-home/story-home.component';
import { CreateEditorComponent } from './create/create-editor/create-editor.component';
import { DirectConnectBackendComponent } from './servers/direct/direct-connect-backend.component';
import { FiltersService } from './servers/filters.service';

const localePrefix = (environment.web) ? '/' : './';

// from the docs
@Injectable() export class HttpTranslationLoader implements L10nTranslationLoader {

	private headers = new HttpHeaders({ 'Content-Type': 'application/json' });

	constructor(
		@Optional() private http: HttpClient,
		@Optional() private injector: Injector) { }

    public get(language: string, provider: L10nProvider): Observable<{ [key: string]: any }> {
		const langs = Languages.toList().filter(a => a.locale.language.startsWith(language + '-'));

		if (langs.length > 0) {
			language = langs[0].locale.language;
		}

		const url = `${provider.asset}-${language.replace(/-/g, '_')}.json`;
		const defTranslationUrl = `${provider.asset}-en.json`;
		const options = {
			headers: this.headers
		};

		this.http
			.get(defTranslationUrl, options)
			.pipe(take(1))
			.subscribe(en => {
				this.injector.get(L10nTranslationService).data = { en };
			});

		return this.http.get(url, options);
	}

}

@Injectable() export class MissingTranslationHandler implements L10nMissingTranslationHandler {

	private translation: L10nTranslationService;
	private inTranslation = false;

	constructor(@Optional() private injector: Injector) { }

	public handle(key: string): string | any {
		if (!this.translation) {
			this.translation = this.injector.get(L10nTranslationService);
		}

		if (this.inTranslation) {
			return key;
		}

		this.inTranslation = true;

		try {
			return this.translation.translate(key, null, 'en');
		} finally {
			this.inTranslation = false;
		}
	}
}

const l10nConfig: L10nConfig = {
	format: 'language-region',
	providers: [
		{ name: 'app', asset: localePrefix + 'assets/languages/locale' }
	],
	fallback: true,
	defaultLocale: { language: 'en' },
	schema: Languages.toList(),
	keySeparator: '.',
};

function initL10n(l10nLoader: L10nLoader): () => Promise<void> {
	return () => l10nLoader.init();
}

export function localStorageFactory() {
	return (typeof window !== 'undefined') ? window.localStorage : null;
}

export function metaFactory(): MetaLoader {
	return new MetaStaticLoader({
		pageTitlePositioning: PageTitlePositioning.PrependPageTitle,
		pageTitleSeparator: ' / ',
		applicationName: 'FiveM server list'
	});
}

@NgModule({
	declarations: [
		SpinnerComponent,
		AppComponent,
		AppNavComponent,
		ConnectingPopupComponent,
		ChangelogPopupComponent,
		ChangelogEntryComponent,
		HomeComponent,
		HomeTweetComponent,
		SettingsComponent,
		ServersComponent,
		ServersContainerComponent,
		ServersListComponent,
		ServersListItemComponent,
		ServerFilterComponent,
		ServerTagFilterComponent,
		ServersDetailComponent,
		DirectConnectComponent,
		DirectConnectBackendComponent,
		PlayerAvatarComponent,
		ColorizePipe,
		EscapePipe,
		ModsComponent,
		ModListComponent,
		ModDetailComponent,
		ModsListComponent,
		ModItemComponent,
		CreateHomeComponent,
		StoryHomeComponent,
		CreateEditorComponent,
	],
	imports: [
		NgDompurifyModule,
		BrowserModule.withServerTransition({ appId: 'cfx-ui' }),
		FormsModule,
		NgSelectModule,
		AppRoutingModule,

		ScrollingModule,
		ExperimentalScrollingModule,

		MomentModule,
		HttpClientModule,
		L10nTranslationModule.forRoot(l10nConfig, {
			translationLoader: HttpTranslationLoader,
			missingTranslationHandler: MissingTranslationHandler,
		}),
		Angulartics2Module.forRoot(),
		LinkyModule,
		MetaModule.forRoot({
			provide: MetaLoader,
			useFactory: (metaFactory)
		}),
		BrowserAnimationsModule,
		MatTabsModule,
		MatSelectModule,
		MatCheckboxModule,
		NgxFilesizeModule,
		Nl2BrPipeModule,
	],
	providers: [
		FiltersService,
		ServersService,
		ServerTagsService,
		TweetService,
		ChangelogService,
		{
			provide: GameService,
			useClass: ((environment.production && !environment.web) || environment.game)
				? CfxGameService
				: DummyGameService
		},
		TrackingService,
		DiscourseService,
		{
			provide: LocalStorage,
			useFactory: (localStorageFactory)
		},
		SettingsService,
		{
			provide: APP_INITIALIZER,
			useFactory: initL10n,
			deps: [L10nLoader],
			multi: true
		},
		ModsService,
	],
	bootstrap: [
		AppComponent
	]
})
export class AppModule {
}
