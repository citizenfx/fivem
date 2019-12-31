import {NgModule} from '@angular/core';
import {Routes, RouterModule} from '@angular/router';

import {HomeComponent} from './home/home.component';
import {SettingsComponent} from './settings/settings.component';
import {MinModeComponent} from './minmode/minmode.component';
import {ServersComponent} from './servers/components/servers.component';
import {ServersDetailComponent} from './servers/components/servers-detail.component';
import {ServersContainerComponent} from './servers/components/servers-container.component';
import {DirectConnectComponent} from './servers/direct/direct-connect.component';

import { environment } from '../environments/environment';
import { MetaGuard } from '@ngx-meta/core';

const routes: Routes = [
	{
		path:      '',
		component: (environment.web) ? ServersContainerComponent : HomeComponent,
		data: 	   { type: 'browse', meta: { title: 'FiveM' } },
		canActivateChild: [MetaGuard],
	},
	{
		path:      'servers',
		component: ServersComponent,
		data: {
			meta: { title: 'Servers' }
		},
		canActivateChild: [MetaGuard],
		children:  [
			{
				path:      '',
				component: ServersContainerComponent,
				data:      {type: 'browse', meta: { title: 'Servers' }}
			},
			{
				path:      'favorites',
				component: ServersContainerComponent,
				data:      {type: 'favorites', meta: { title: 'Favorites' }}
			},
			{
				path:      'history',
				component: ServersContainerComponent,
				data:      {type: 'history', meta: { title: 'History' }}
			},
			{
				path:      'premium',
				component: ServersContainerComponent,
				data:      {type: 'premium', meta: { title: 'Premium' }}
			},
			{
				path:      'direct-connect',
				component: DirectConnectComponent
			},
			{
				path:	   'detail/:addr',
				component: ServersDetailComponent
			}
		]
	},
	{
		path:      'settings',
		component: SettingsComponent
	},
	{
		path:      'minmode',
		component: MinModeComponent
	}
];

@NgModule({
	imports: [RouterModule.forRoot(routes, { useHash: !environment.web })],
	exports: [RouterModule]
})
export class AppRoutingModule {
}
