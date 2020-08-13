import { Component, Input, ViewChild, EmbeddedViewRef, ComponentFactoryResolver, ApplicationRef, Injector, AfterViewInit, OnDestroy, ViewContainerRef, TemplateRef, OnInit } from '@angular/core';
import { GameService } from '../game.service';
import { Tweet } from './tweet.service';
import { DomPortalOutlet, TemplatePortal } from '@angular/cdk/portal';

@Component({
	moduleId: module.id,
	selector: 'app-home-tweet',
	templateUrl: 'home-tweet.component.html',
	styleUrls: ['./home-tweet.component.scss']
})
export class HomeTweetComponent implements AfterViewInit, OnInit, OnDestroy {
	@ViewChild('cdkPortal')
	portal: TemplateRef<any>;

	private embeddedViewRef: EmbeddedViewRef<any>;

	@Input()
	public tweet: Tweet;

	@Input()
	public actuallyTweet: boolean;

	zoomedImageUrl = '';

	constructor(
		private gameService: GameService,
		private cfr: ComponentFactoryResolver,
		private ar: ApplicationRef,
		private injector: Injector,
		private vcr: ViewContainerRef,
	) { }

	handleEsc = (event: KeyboardEvent) => {
		if (event.key === 'Escape' && this.zoomedImageUrl) {
			this.zoomedImageUrl = undefined;
		}
	};

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
				return false;
			}
		}

		return false;
	}

	zoomImage() {
		this.zoomedImageUrl = this.tweet.image;
	}

	ngAfterViewInit() {
		this.embeddedViewRef = new DomPortalOutlet(
			document.getElementById('overlays'),
			this.cfr,
			this.ar,
			this.injector,
		).attach(new TemplatePortal(this.portal, this.vcr));
	}

	ngOnInit() {
		document.addEventListener('keydown', this.handleEsc);
	}

	ngOnDestroy() {
		this.embeddedViewRef.destroy();

		document.removeEventListener('keydown', this.handleEsc);
	}
}
