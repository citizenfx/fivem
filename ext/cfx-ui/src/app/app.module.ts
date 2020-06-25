import {BrowserModule} from '@angular/platform-browser';
import {BrowserAnimationsModule} from '@angular/platform-browser/animations';
import {NgModule, Inject, Injectable, Optional, APP_INITIALIZER, Injector} from '@angular/core';
import {FormsModule} from '@angular/forms';
import {HttpClientModule, HttpHeaders, HttpClient, HttpParams} from '@angular/common/http';

import { environment } from '../environments/environment'

import {VirtualScrollerModule} from 'ngx-virtual-scroller';
import {MomentModule} from 'angular2-moment';
import {Angulartics2Module} from 'angulartics2';
import {Angulartics2Piwik} from 'angulartics2/piwik';
import {LinkyModule} from 'ngx-linky';
import {NgxFilesizeModule} from 'ngx-filesize';
import {MetaModule, MetaLoader, MetaStaticLoader, PageTitlePositioning} from '@ngx-meta/core';

import {AppRoutingModule} from './app-routing.module';
import {AppComponent} from './app.component';

import {AppNavComponent} from './app-nav.component';
import {ConnectingPopupComponent} from './servers/connecting-popup.component';
import {HomeComponent} from './home/home.component';
import {HomeTweetComponent} from './home/home-tweet.component';
import {SettingsComponent} from './settings/settings.component';
import {MinModeComponent} from './minmode/minmode.component';

import {ServersComponent} from './servers/components/servers.component';
import {ServersContainerComponent} from './servers/components/servers-container.component';
import {ServersListComponent} from './servers/components/servers-list.component';
import {ServersListItemComponent} from './servers/components/servers-list-item.component';
import {ServersListHeaderComponent} from './servers/components/servers-list-header.component';
import {ServerFilterComponent} from './servers/components/server-filter.component';
import {ServerTagFilterComponent} from './servers/components/server-tag-filter.component';
import {ServersDetailComponent} from './servers/components/servers-detail.component';
import {PlayerAvatarComponent} from './servers/components/player-avatar.component';
import {DirectConnectComponent} from './servers/direct/direct-connect.component';

import {ServersService} from './servers/servers.service';
import {TweetService} from './home/tweet.service';
import {TrackingService} from './tracking.service';
import {SettingsService} from './settings.service';

import {GameService, CfxGameService, DummyGameService} from './game.service';
import {DiscourseService} from './discourse.service';

import {ColorizePipe} from './colorize.pipe';
import {EscapePipe} from './escape.pipe';
import { LocalStorage } from './local-storage';

import { Languages } from './languages';
import { ServerTagsService } from './servers/server-tags.service';
import { ChangelogPopupComponent } from './home/app-changelog-popup.component';
import { ChangelogEntryComponent } from './home/app-changelog-entry.component';
import { ChangelogService } from './changelogs.service';
import { L10nConfig, L10nTranslationLoader, L10nProvider, L10nLoader, L10nTranslationModule,
	L10nMissingTranslationHandler, L10nTranslationService } from 'angular-l10n';
import { Observable } from 'rxjs';
import { take } from 'rxjs/operators';

import { MatTabsModule } from '@angular/material/tabs';
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

const localePrefix = (environment.web) ? '/' : './';

// from the docs
@Injectable() export class HttpTranslationLoader implements L10nTranslationLoader {

    private headers = new HttpHeaders({ 'Content-Type': 'application/json' });

	constructor(
		@Optional() private http: HttpClient,
		@Optional() private injector: Injector) { }

    public get(language: string, provider: L10nProvider): Observable<{ [key: string]: any }> {
		const url = `${provider.asset}-${language}.json`;
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
	format: 'language',
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
		ServersListHeaderComponent,
		ServerFilterComponent,
		ServerTagFilterComponent,
		ServersDetailComponent,
		DirectConnectComponent,
		DirectConnectBackendComponent,
		PlayerAvatarComponent,
		MinModeComponent,
		ColorizePipe,
		EscapePipe,
		ModsComponent,
		ModListComponent,
		ModDetailComponent,
		ModsListComponent,
		ModItemComponent,
		CreateHomeComponent,
		StoryHomeComponent,
		CreateEditorComponent
	],
	imports:      [
		BrowserModule.withServerTransition({ appId: 'cfx-ui' }),
		FormsModule,
		AppRoutingModule,
		VirtualScrollerModule,
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
		NgxFilesizeModule,
	],
	providers:    [
		ServersService,
		ServerTagsService,
		TweetService,
		ChangelogService,
		{
			provide:  GameService,
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
	bootstrap:    [
		AppComponent
	]
})
export class AppModule {
}
