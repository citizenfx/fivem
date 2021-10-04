import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { NgModule, Injectable, Optional, APP_INITIALIZER, Injector, Inject, ErrorHandler } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { HttpClientModule, HttpHeaders, HttpClient } from '@angular/common/http';
import { NgSelectModule } from '@ng-select/ng-select';
import { Nl2BrPipeModule } from 'nl2br-pipe';
import { NgxMasonryModule } from 'ngx-masonry';
import { Router } from '@angular/router';
import * as Sentry from '@sentry/angular';

import { NgDompurifyModule } from '@tinkoff/ng-dompurify';

import { environment } from '../environments/environment'

import { ScrollingModule } from '@angular/cdk/scrolling';
import { ScrollingModule as ExperimentalScrollingModule } from '@angular/cdk-experimental/scrolling';

import { DateFnsConfigurationService } from 'ngx-date-fns';
import {
  FormatDistanceToNowPipeModule,
  FormatDurationPipeModule,
  ParsePipeModule
} from 'ngx-date-fns';

import { Angulartics2Module } from 'angulartics2';
import { LinkyModule } from 'ngx-linky';
import { NgxFilesizeModule } from 'ngx-filesize';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';

import { AppNavComponent } from './nav/app-nav.component';
import { ConnectingPopupComponent } from './servers/connecting-popup.component';
import { ConnectionBoundaryComponent } from './servers/connection-boundary.component';
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
import { fromEntries, SettingsService } from './settings.service';

import { GameService, CfxGameService, DummyGameService } from './game.service';
import { DiscourseService } from './discourse.service';

import { ColorizePipe } from './colorize.pipe';
import { EscapePipe } from './escape.pipe';
import { SafeHtmlPipe } from './safe.pipe';
import { AllowifyPipe } from './allowify.pipe';
import { RejectifyPipe } from './rejectify.pipe';
import { LocalStorage } from './local-storage';

import { Languages } from './languages';
import { ServerTagsService } from './servers/server-tags.service';
import { ChangelogPopupComponent } from './changelog/app-changelog-popup.component';
import { ChangelogEntryComponent } from './changelog/app-changelog-entry.component';
import { ChangelogService } from './changelogs.service';
import {
	L10nConfig, L10nTranslationLoader, L10nProvider, L10nLoader, L10nTranslationModule,
	L10nTranslationFallback, L10nTranslationService, L10N_CONFIG, L10nCache
} from 'angular-l10n';
import { Observable } from 'rxjs';
import languageRefs from 'webpack-extended-import-glob-loader!./languagerefs';

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
import { AuthModalComponent } from './auth-modal/auth-modal.component';
import { ServerReviewsComponent } from './servers/components/reviews/server-reviews.component';
import { ForumSignoutInterceptorState, httpInterceptorProviders } from './http-interceptors';
import { ServerReviewComponent } from './servers/components/reviews/server-review.component';

@Injectable() export class TranslationFallback implements L10nTranslationFallback {

	constructor(
		@Inject(L10N_CONFIG) private config: L10nConfig,
		private cache: L10nCache,
		private translationLoader: L10nTranslationLoader
	) { }

    get(language: string, provider: L10nProvider): Observable<any>[] {
        const loaders: Observable<any>[] = [];

        if (this.config.cache) {
            loaders.push(
                this.cache.read(`${provider.name}-fallback-${language}`,
                    this.translationLoader.get('en', provider)));
            loaders.push(
                this.cache.read(`${provider.name}-${language}`,
                    this.translationLoader.get(language, provider)));
        } else {
            loaders.push(this.translationLoader.get('en', provider));
            loaders.push(this.translationLoader.get(language, provider));
        }

        return loaders;
    }
}

const l10nConfig: L10nConfig = {
	format: 'language-region',
	providers: [
		{ name: 'app', asset: fromEntries((languageRefs as any[]).map(lang =>
			[ (lang.fileName as string).replace(/.*locale-(.*?)\.json/g, '$1').replace(/_/g, '-'), lang.module.default ]
		)) }
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

@NgModule({
	declarations: [
		SpinnerComponent,
		AppComponent,
		AppNavComponent,
		ConnectingPopupComponent,
        ConnectionBoundaryComponent,
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
		ServerReviewComponent,
		ServerReviewsComponent,
		DirectConnectComponent,
		DirectConnectBackendComponent,
		PlayerAvatarComponent,
		ColorizePipe,
		EscapePipe,
		SafeHtmlPipe,
		AllowifyPipe,
		RejectifyPipe,
		ModsComponent,
		ModListComponent,
		ModDetailComponent,
		ModsListComponent,
		ModItemComponent,
		CreateHomeComponent,
		StoryHomeComponent,
		CreateEditorComponent,
		AuthModalComponent
	],
	imports: [
		NgDompurifyModule,
		BrowserModule.withServerTransition({ appId: 'cfx-ui' }),
		FormsModule,
		NgSelectModule,
		AppRoutingModule,

		ScrollingModule,
		ExperimentalScrollingModule,

		FormatDistanceToNowPipeModule,
		FormatDurationPipeModule,
		ParsePipeModule,

		HttpClientModule,
		L10nTranslationModule.forRoot(l10nConfig, {
			translationFallback: TranslationFallback
		}),
		Angulartics2Module.forRoot(),
		LinkyModule,
		BrowserAnimationsModule,
		NgxFilesizeModule,
		Nl2BrPipeModule,
		NgxMasonryModule,
	],
	providers: [
		{
			provide: ErrorHandler,
			useValue: Sentry.createErrorHandler({
				showDialog: false,
			}),
		},
		{
			provide: Sentry.TraceService,
			deps: [Router],
		},
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
			// L10nLoader must be first so that initL10n will be called with the right arg
			deps: [L10nLoader, Sentry.TraceService],
			multi: true
		},
		ModsService,
		DateFnsConfigurationService,
		ForumSignoutInterceptorState,
		httpInterceptorProviders,
	],
	bootstrap: [
		AppComponent
	]
})
export class AppModule {
}
