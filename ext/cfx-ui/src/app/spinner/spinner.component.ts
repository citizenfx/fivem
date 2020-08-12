import { Component } from "@angular/core";
import { GameService } from "../game.service";
import { DomSanitizer } from "@angular/platform-browser";

@Component({
    moduleId: module.id,
	selector: 'cfx-spinner',
    templateUrl: 'spinner.component.html',
	styleUrls:   ['./spinner.component.scss']
})
export class SpinnerComponent {
	spinnerStyle: any;

	constructor(
		private gameService: GameService,
		private sanitizer: DomSanitizer,
	) {
		this.spinnerStyle = sanitizer.bypassSecurityTrustStyle(`clip-path: url(#clip${gameService.gameName})`);
	}
}
