import { Component } from '@angular/core';

@Component({
  selector: 'app-root',
  template: `
    <router-outlet></router-outlet>
    <app-bottom-nav></app-bottom-nav>
  `
})
export class AppComponent {
  title = 'app works!';
}
