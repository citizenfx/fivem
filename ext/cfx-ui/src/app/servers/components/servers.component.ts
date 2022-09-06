import {Component} from '@angular/core';

@Component({
	moduleId: module.id,
	selector: 'servers',
	template: '<router-outlet></router-outlet>',
	styles: [':host { overflow: hidden; height: 100% }'],
})
export class ServersComponent {}
