import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { HttpModule } from '@angular/http';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';

import { BottomNavComponent } from './bottom-nav.component';
import { HomeComponent } from './home.component';
import { ServerListComponent } from './server-list.component';

@NgModule({
  declarations: [
    AppComponent,
    BottomNavComponent,
    HomeComponent,
    ServerListComponent
  ],
  imports: [
    BrowserModule,
    FormsModule,
    HttpModule,
    AppRoutingModule
  ],
  providers: [],
  bootstrap: [
    AppComponent
  ]
})
export class AppModule { }
