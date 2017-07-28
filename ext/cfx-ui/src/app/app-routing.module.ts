import {NgModule} from '@angular/core';
import {Routes, RouterModule} from '@angular/router';

import {HomeComponent} from './home/home.component';
import {SettingsComponent} from './settings/settings.component';
import {ServersComponent} from './servers/components/servers.component';
import {ServersContainerComponent} from './servers/components/servers-container.component';
import {DirectConnectComponent} from './servers/direct/direct-connect.component';

const routes: Routes = [
	{
		path:      '',
		component: HomeComponent
	},
	{
		path:      'servers',
		component: ServersComponent,
		children:  [
			{
				path:      '',
				component: ServersContainerComponent,
				data:      {type: 'browse'}
			},
			{
				path:      'favorites',
				component: ServersContainerComponent,
				data:      {type: 'favorites'}
			},
			{
				path:      'history',
				component: ServersContainerComponent,
				data:      {type: 'history'}
			},
			{
				path:      'direct-connect',
				component: DirectConnectComponent
			}
		]
	},
	{
		path:      'settings',
		component: SettingsComponent
	}
];

@NgModule({
	imports: [RouterModule.forRoot(routes, { useHash: true })],
	exports: [RouterModule]
})
export class AppRoutingModule {
}
