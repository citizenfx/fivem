import { Component, OnInit, ElementRef, ViewChild, Output, EventEmitter, Inject } from '@angular/core';
import { GameService } from '../game.service';
import * as AdaptiveCards from 'adaptivecards';
import { L10N_LOCALE, L10nLocale } from 'angular-l10n';
import { ActionAlignment } from 'adaptivecards';

// Matches `<ip>:<port>` and `<ip> port <port>`
const ipRegex = Array(4).fill('(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)').join('\\.');
const serverAddressRegex = new RegExp(String.raw`\b${ipRegex}(\s+port\s+|:)\d+\b`, 'g');

@Component({
	moduleId: module.id,
	selector: 'connecting-popup',
	templateUrl: 'connecting-popup.component.html',
	styleUrls: ['connecting-popup.component.scss']
})
export class ConnectingPopupComponent implements OnInit {
	showOverlay = false;
	overlayClosable = true;
	overlayTitle: string;
	overlayMessage: string;
	overlayMessageData: { [key: string]: any } = {};
	overlayBg = '';
	closeLabel = "#Servers_CloseOverlay";
	retryLabel = "#Servers_Retry";
	submitting = false;
	closeKeys = ["Escape", "Backspace", "Delete"];

	@Output()
	retry = new EventEmitter();

	@ViewChild('card') cardElement: ElementRef;

	constructor(
		private gameService: GameService,
		@Inject(L10N_LOCALE) public locale: L10nLocale
	) {

	}

	ngOnInit() {
		this.gameService.connecting.subscribe(a => {
			this.overlayTitle = '#Servers_Connecting';
			this.overlayMessage = this.gameService.streamerMode
				? '#Servers_ConnectingToServer'
				: '#Servers_ConnectingTo';
			this.overlayMessageData = {
				serverName: a?.address || 'unknown',
			};
			this.showOverlay = true;
			this.overlayClosable = false;

			this.overlayBg = a?.data?.vars?.banner_connecting || '';

			this.clearElements();
		});

		this.gameService.connectFailed.subscribe(([server, message]) => {
			this.overlayTitle = '#Servers_ConnectFailed';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = { message };
			this.showOverlay = true;
			this.overlayClosable = true;
			this.closeLabel = "#Servers_CloseOverlay";

			if (this.gameService.streamerMode && message) {
				this.overlayMessageData.message = message.replace(serverAddressRegex, '&lt;HIDDEN&gt;');
			}

			this.clearElements();
		});

		this.gameService.connectStatus.subscribe(a => {
			this.overlayTitle = '#Servers_Connecting';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = {
				message: a.message,
				serverName: this.gameService.minmodeBlob.productName,
			};
			this.showOverlay = true;
			this.overlayClosable = (a.count == 133 && a.total == 133); // magic numbers, yeah :(

			if (this.overlayClosable) {
				this.closeLabel = "#Servers_CancelOverlay";
			}

			if (this.gameService.streamerMode && a.message) {
				this.overlayMessageData.message = a.message.replace(serverAddressRegex, '&lt;HIDDEN&gt;');
			}

			this.clearElements();
		});

		this.gameService.connectCard.subscribe(a => {
			const getSize = (size: number) => {
				return (size / 720) * Math.min(window.innerHeight, window.innerWidth);
			}

			this.showOverlay = true;

			const adaptiveCard = new AdaptiveCards.AdaptiveCard();
			adaptiveCard.hostConfig = new AdaptiveCards.HostConfig({
				spacing: {
				},
				supportsInteractivity: true,
				actions: {
					actionsOrientation: 'horizontal',
					actionAlignment: ActionAlignment.Stretch,
				},
				fontTypes: {
					default: {
						fontFamily: 'Rubik',
						fontSizes: {
							small: getSize(8),
							default: getSize(10),
							medium: getSize(13),
							large: getSize(17),
							extraLarge: getSize(22),
						}
					},
					monospace: {
						fontSizes: {
							small: getSize(8),
							default: getSize(10),
							medium: getSize(13),
							large: getSize(17),
							extraLarge: getSize(22),
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
			this.overlayMessageData = { message };
			this.showOverlay = true;
			this.overlayClosable = true;
			this.closeLabel = "#Servers_CloseOverlay";

			this.clearElements();
		});

		this.gameService.infoMessage.subscribe(message => {
			this.overlayTitle = '#Servers_Info';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = { message };
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

	clickContent(event: MouseEvent) {
		const srcElement = event.srcElement as HTMLElement;

		if (srcElement.localName === 'a') {
			this.gameService.openUrl(srcElement.getAttribute('href'));

			event.preventDefault();
		}
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

	onKeyPress(event: KeyboardEvent) {
		if (this.closeKeys.includes(event.code)) {
			this.closeOverlay()
		}
	}
}
