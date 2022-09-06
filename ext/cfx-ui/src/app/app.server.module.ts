import {NgModule} from '@angular/core';
import {ServerModule} from '@angular/platform-server';
import {ModuleMapLoaderModule} from '@nguniversal/module-map-ngfactory-loader';

import {AppModule} from './app.module';
import {AppComponent} from './app.component';
import { LocalStorage } from './local-storage';

@NgModule({
  imports: [
    AppModule,
    ServerModule,
    ModuleMapLoaderModule
  ],
  providers: [
      {
          provide: LocalStorage,
          useValue: {
              getItem() {

              }
          }
      }
  ],
  bootstrap: [AppComponent],
})
export class AppServerModule {}
