import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';
import {FormsModule} from '@angular/forms';
import {HttpModule} from '@angular/http';
import {HttpClientModule} from '@angular/common/http';

import { environment } from '../environments/environment'

import {VirtualScrollerModule} from 'ngx-virtual-scroller';
import {TranslationModule, L10nConfig, L10nLoader, ProviderType} from 'angular-l10n';
import {MomentModule} from 'angular2-moment';
import {Angulartics2Module} from 'angulartics2';
import {Angulartics2Piwik} from 'angulartics2/piwik';
import {LinkyModule} from 'ngx-linky';

import {AppRoutingModule} from './app-routing.module';
import {AppComponent} from './app.component';

import {AppNavComponent} from './app-nav.component';
import {ConnectingPopupComponent} from './servers/connecting-popup.component';
import {HomeComponent} from './home/home.component';
import {SettingsComponent} from './settings/settings.component';

import {ServersComponent} from './servers/components/servers.component';
import {ServersContainerComponent} from './servers/components/servers-container.component';
import {ServersListComponent} from './servers/components/servers-list.component';
import {ServersListItemComponent} from './servers/components/servers-list-item.component';
import {ServersListHeaderComponent} from './servers/components/servers-list-header.component';
import {ServerFilterComponent} from './servers/components/server-filter.component';
import {ServersDetailComponent} from './servers/components/servers-detail.component';
import {PlayerAvatarComponent} from './servers/components/player-avatar.component';
import {DirectConnectComponent} from './servers/direct/direct-connect.component';

import {ServersService} from './servers/servers.service';
import {TweetService} from './home/tweet.service';
import {TrackingService} from './tracking.service';

import {GameService, CfxGameService, DummyGameService} from './game.service';

import {ColorizePipe} from './colorize.pipe';
import {EscapePipe} from './escape.pipe';

const l10nConfig: L10nConfig = {
	locale: {
		languages: [
			{ code: 'en', dir: 'ltr' }
		],
		language: 'en'
	},
	translation: {
		providers: [
			{ type: ProviderType.Static, prefix: './assets/locale-' }
		]
	}
};

@NgModule({
	declarations: [
		AppComponent,
		AppNavComponent,
		ConnectingPopupComponent,
		HomeComponent,
		SettingsComponent,
		ServersComponent,
		ServersContainerComponent,
		ServersListComponent,
		ServersListItemComponent,
		ServersListHeaderComponent,
		ServerFilterComponent,
		ServersDetailComponent,
		DirectConnectComponent,
		PlayerAvatarComponent,
		ColorizePipe,
		EscapePipe
	],
	imports:      [
		BrowserModule,
		FormsModule,
		HttpModule,
		AppRoutingModule,
		VirtualScrollerModule,
		MomentModule,
		HttpClientModule,
		TranslationModule.forRoot(l10nConfig),
		Angulartics2Module.forRoot(),
		LinkyModule,
	],
	providers:    [
		ServersService,
		TweetService,
		{
			provide:  GameService,
			useClass: (environment.production && !environment.web)
				? CfxGameService
				: DummyGameService
		},
		TrackingService
	],
	bootstrap:    [
		AppComponent
	]
})
export class AppModule {
	constructor(public l10nLoader: L10nLoader) {
		this.l10nLoader.load();
	}
}
