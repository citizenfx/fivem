import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { HomeComponent } from './home/home.component';
import { SettingsComponent } from './settings/settings.component';
import { ServersComponent } from './servers/components/servers.component';
import { ServersDetailComponent } from './servers/components/detail/servers-detail.component';
import { ServersContainerComponent } from './servers/components/servers-container.component';
import { DirectConnectComponent } from './servers/direct/direct-connect.component';

import { environment } from '../environments/environment';
import { MetaGuard } from '@ngx-meta/core';
import { ModsComponent } from './mods/mods/mods.component';
import { ModListComponent } from './mods/mod-list/mod-list.component';
import { ModDetailComponent } from './mods/mod-detail/mod-detail.component';
import { ChangelogPopupComponent } from './changelog/app-changelog-popup.component';
import { CreateHomeComponent } from './create/create-home/create-home.component';
import { CreateEditorComponent } from './create/create-editor/create-editor.component';
import { StoryHomeComponent } from './story/story-home/story-home.component';

const routes: Routes = [
	{
		path: '',
		component: (environment.web) ? ServersContainerComponent : HomeComponent,
		data: { type: 'browse', meta: { title: 'FiveM' } },
		canActivateChild: [MetaGuard],
	},
	{
		path: 'home',
		component: HomeComponent,
		canActivateChild: [MetaGuard],
	},
	{
		path: 'changelog',
		component: ChangelogPopupComponent,
	},
	{
		path: 'servers',
		component: ServersComponent,
		data: {
			meta: { title: 'Servers' }
		},
		canActivateChild: [MetaGuard],
		children: [
			{
				path: '',
				component: ServersContainerComponent,
				data: { type: 'browse', meta: { title: 'Servers' } }
			},
			{
				path: 'favorites',
				component: ServersContainerComponent,
				data: { type: 'favorites', meta: { title: 'Favorites' } }
			},
			{
				path: 'history',
				component: ServersContainerComponent,
				data: { type: 'history', meta: { title: 'History' } }
			},
			{
				path: 'premium',
				component: ServersContainerComponent,
				data: { type: 'premium', meta: { title: 'Premium' } }
			},
			{
				path: 'direct-connect',
				component: DirectConnectComponent
			},
			{
				path: 'detail/:addr',
				component: ServersDetailComponent
			}
		]
	},
	{
		path: 'mods',
		component: ModsComponent,
		children: [
			{
				path: '',
				component: ModListComponent
			},
			{
				path: 'detail/:id',
				component: ModDetailComponent
			}
		]
	},
	{
		path: 'create',
		component: CreateHomeComponent,
	},
	{
		path: 'editor',
		component: CreateEditorComponent,
	},
	{
		path: 'story',
		component: StoryHomeComponent,
	},
	{
		path: 'settings',
		component: SettingsComponent
	}
];

@NgModule({
	imports: [RouterModule.forRoot(routes, { useHash: !environment.web })],
	exports: [RouterModule]
})
export class AppRoutingModule {
}
