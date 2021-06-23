import { Component, Input } from '@angular/core';

@Component({
	moduleId: module.id,
	selector: 'app-connection-boundary',
	templateUrl: 'connection-boundary.component.html',
	styleUrls: ['connection-boundary.component.scss']
})
export class ConnectionBoundaryComponent {
    @Input()
    public state: 'broken' | 'ok' | 'unknown' | 'bye';
}
