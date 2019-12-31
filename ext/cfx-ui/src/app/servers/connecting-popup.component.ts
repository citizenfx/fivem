import { Component, OnInit, ElementRef, ViewChild, Input, Output, EventEmitter } from '@angular/core';
import { GameService } from '../game.service';
import { Translation, TranslationService } from 'angular-l10n';
import * as AdaptiveCards from 'adaptivecards';

@Component({
	moduleId:    module.id,
	selector:    'connecting-popup',
	templateUrl: 'connecting-popup.component.html',
	styleUrls:   ['connecting-popup.component.scss']
})
export class ConnectingPopupComponent extends Translation implements OnInit {
	showOverlay = false;
	overlayClosable = true;
	overlayTitle: string;
	overlayMessage: string;
	overlayMessageData = {};
	overlayBg = '';
	closeLabel = "#Servers_CloseOverlay";
	retryLabel = "#Servers_Retry";
	submitting = false;

	@Input()
	minMode = false;

	@Output()
	retry = new EventEmitter();

	@ViewChild('card') cardElement: ElementRef;

	constructor(
		private gameService: GameService,
		public translation: TranslationService
	) {
		super();
	}

	ngOnInit() {
		this.gameService.connecting.subscribe(a => {
			this.overlayTitle = (!this.minMode) ? '#Servers_Connecting' : '#Servers_ConnectingTo';
			this.overlayMessage = '#Servers_ConnectingTo';
			this.overlayMessageData = {serverName: this.minMode
				? this.gameService.minmodeBlob.productName
				: ((a) ? a.address : 'unknown')};
			this.showOverlay = true;
			this.overlayClosable = false;

			this.overlayBg = (a && a.data && a.data.vars && a.data.vars.banner_connecting) ? a.data.vars.banner_connecting : '';

			this.clearElements();
		});

		this.gameService.connectFailed.subscribe(([server, message]) => {
			this.overlayTitle = '#Servers_ConnectFailed';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = {message};
			this.showOverlay = true;
			this.overlayClosable = true;
			this.closeLabel = "#Servers_CloseOverlay";

			this.clearElements();
		});

		this.gameService.connectStatus.subscribe(a => {
			this.overlayTitle = (!this.minMode) ? '#Servers_Connecting' : '#Servers_ConnectingTo';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = {message: a.message, serverName: this.gameService.minmodeBlob.productName};
			this.showOverlay = true;
			this.overlayClosable = (a.count == 133 && a.total == 133); // magic numbers, yeah :(

			if (this.overlayClosable) {
				this.closeLabel = "#Servers_CancelOverlay";
			}

			this.clearElements();
		});

		this.gameService.connectCard.subscribe(a => {
			const getSize = (size: number) => {
				return (size / 720) * window.innerHeight;
			}

			this.showOverlay = true;

			const adaptiveCard = new AdaptiveCards.AdaptiveCard();
			adaptiveCard.hostConfig = new AdaptiveCards.HostConfig({
				fontFamily: 'Segoe UI, Helvetica Neue, sans-serif',
				spacing: {
					padding: 0
				},
				supportsInteractivity: true,
				fontTypes: {
					default: {
						fontSizes: {
							small: getSize(12),
							default: getSize(14),
							medium: getSize(17),
							large: getSize(21),
							extraLarge: getSize(26),
						}
					},
					monospace: {
						fontSizes: {
							small: getSize(12),
							default: getSize(14),
							medium: getSize(17),
							large: getSize(21),
							extraLarge: getSize(26),
						}
					}
				},
				imageSizes: {
					small: 40,
					medium: 80,
					large: 120
				},
				imageSet: {
					imageSize: 'medium',
					maxImageHeight: 100
				},
				containerStyles: {
					default: {
						foregroundColors: {
							default: {
								default: '#FFFFFF',
								subtle: '#88FFFFFF',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							dark: {
								default: '#000000',
								subtle: '#66000000',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							light: {
								default: '#FFFFFF',
								subtle: '#33000000',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							accent: {
								default: '#2E89FC',
								subtle: '#882E89FC',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							good: {
								default: '#00FF00',
								subtle: '#DD00FF00',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							warning: {
								default: '#FFD800',
								subtle: '#DDFFD800',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							attention: {
								default: '#FF0000',
								subtle: '#DDFF0000',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							}
						},
						backgroundColor: '#00000000'
					},
					emphasis: {
						foregroundColors: {
							default: {
								default: '#FFFFFF',
								subtle: '#88FFFFFF',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							dark: {
								default: '#000000',
								subtle: '#66000000',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							light: {
								default: '#FFFFFF',
								subtle: '#33000000',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							accent: {
								default: '#2E89FC',
								subtle: '#882E89FC',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							good: {
								default: '#00FF00',
								subtle: '#DD00FF00',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							warning: {
								default: '#FF0000',
								subtle: '#DDFF0000',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							},
							attention: {
								default: '#FFD800',
								subtle: '#DDFFD800',
								highlightColors: {
									default: '#22000000',
									subtle: '#11000000'
								}
							}
						},
						backgroundColor: '#00000000'
					}
				}
			});

			adaptiveCard.onExecuteAction = (action) => {
				if (action instanceof AdaptiveCards.SubmitAction) {
					if (!this.submitting) {
						let data = action.data;

						if (action.id) {
							data = { ...action.data, submitId: action.id };
						}

						this.gameService.submitCardResponse(data);
					}

					this.submitting = true;
				} else if (action instanceof AdaptiveCards.OpenUrlAction) {
					this.gameService.openUrl(action.url);
				}
			};

			adaptiveCard.parse(JSON.parse(a.card));

			const renderedCard = adaptiveCard.render();
			const element = this.cardElement.nativeElement as HTMLElement;

			element.childNodes.forEach(c => element.removeChild(c));
			element.appendChild(renderedCard);

			this.submitting = false;
			this.overlayMessage = '';
		});

		this.gameService.errorMessage.subscribe(message => {
			this.overlayTitle = '#Servers_Error';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = {message};
			this.showOverlay = true;
			this.overlayClosable = true;
			this.closeLabel = "#Servers_CloseOverlay";

			this.clearElements();
		});

		this.gameService.infoMessage.subscribe(message => {
			this.overlayTitle = '#Servers_Info';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = {message};
			this.showOverlay = true;
			this.overlayClosable = true;
			this.closeLabel = "#Servers_CloseOverlay";

			this.clearElements();
		});
	}

	clearElements() {
		const element = this.cardElement.nativeElement as HTMLElement;

		element.childNodes.forEach(c => element.removeChild(c));
	}

	closeOverlay() {
		if (this.overlayClosable) {
			this.showOverlay = false;

			this.gameService.cancelNativeConnect();
		}
	}

	doRetry() {
		this.retry.emit();
	}
}
