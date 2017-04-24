import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { HomeComponent } from './home/home.component';
import { ServerListComponent } from './servers/ui/server-list.component';
import { ServerListUiComponent } from './servers/ui/server-list-ui.component';

const routes: Routes = [
  {
    path: '',
    component: HomeComponent
  },
  {
    path: 'servers',
    component: ServerListComponent,
    children: [
      {
        path: '',
        component: ServerListUiComponent,
        data: { type: 'browse' }
      },
      {
        path: 'favorites',
        component: ServerListUiComponent,
        data: { type: 'favorites' }
      },
      {
        path: 'history',
        component: ServerListUiComponent,
        data: { type: 'history' }
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
