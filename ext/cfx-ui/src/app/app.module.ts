import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';
import {FormsModule} from '@angular/forms';
import {HttpModule} from '@angular/http';

import { environment } from '../environments/environment'

import {VirtualScrollModule} from 'angular2-virtual-scroll';
import {TranslationModule} from 'angular-l10n';
import {MomentModule} from 'angular2-moment';

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
import {DirectConnectComponent} from './servers/direct/direct-connect.component';

import {ServersService} from './servers/servers.service';
import {TweetService} from './home/tweet.service';

import {GameService, CfxGameService, DummyGameService} from './game.service';

import {ColorizePipe} from './colorize.pipe';
import {EscapePipe} from './escape.pipe';

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
		DirectConnectComponent,
		ColorizePipe,
		EscapePipe
	],
	imports:      [
		BrowserModule,
		FormsModule,
		HttpModule,
		AppRoutingModule,
		VirtualScrollModule,
		MomentModule,
		TranslationModule.forRoot()
	],
	providers:    [
		ServersService,
		TweetService,
		{
			provide:  GameService,
			useClass: environment.production
				? CfxGameService
				: DummyGameService
		}
	],
	bootstrap:    [
		AppComponent
	]
})
export class AppModule {
}
