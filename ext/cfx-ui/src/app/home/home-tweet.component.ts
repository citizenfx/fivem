import { Component, Input } from '@angular/core';
import { GameService } from '../game.service';
import { Tweet } from './tweet.service';

@Component({
	moduleId: module.id,
	selector: 'app-home-tweet',
	templateUrl: 'home-tweet.component.html',
	styleUrls: ['./home-tweet.component.scss']
})

export class HomeTweetComponent {
	@Input()
	public tweet: Tweet;

	@Input()
	public actuallyTweet: boolean;

	constructor(private gameService: GameService) {

	}

	openTweet(id) {
		this.gameService.openUrl(this.tweet.url);
	}

	clickContent(event: MouseEvent) {
		event.preventDefault();

		for (const pathElement of event.composedPath()) {
			const element = (pathElement as HTMLElement);

			if (Array.from(element.classList).includes('__cfx_ap_content')) {
				return false;
			}

			if (element.localName === 'a') {
				this.gameService.openUrl(element.getAttribute('href'));
			}
		}

		return false;
	}
}
