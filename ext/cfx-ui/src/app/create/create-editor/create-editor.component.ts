import { Component } from '@angular/core';

@Component({
	selector: 'app-create-editor',
	templateUrl: './create-editor.component.html',
	styleUrls: ['./create-editor.component.scss']
})
export class CreateEditorComponent {
	clickContent(event: MouseEvent) {
		const srcElement = event.srcElement as HTMLElement;

		if (srcElement.localName === 'button') {
			(<any>window).invokeNative('executeCommand', 'replayEditor');
			event.preventDefault();
		}
	}
}
