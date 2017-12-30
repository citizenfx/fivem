import {Injectable} from '@angular/core';

import {GameService, Profile} from './game.service';
import {Server} from './servers/server'

import {Angulartics2} from 'angulartics2';
import {Angulartics2Piwik} from 'angulartics2/piwik';

@Injectable()
export class TrackingService {
	constructor(private gameService: GameService, private angulartics: Angulartics2, private piwik: Angulartics2Piwik) {
		this.gameService.connecting.subscribe((event: Server) => {
			if (!event) {
				return;
			}

			this.angulartics.eventTrack.next({
				action: 'Connecting',
				properties: {
					category: 'Server',
					label: event.hostname + ' (' + event.address + ')'
				}
			});

			this.angulartics.eventTrack.next({
				action: 'ConnectingRaw',
				properties: {
					category: 'Server',
					label: event.address
				}
			});
		});

		this.gameService.connectFailed.subscribe(([server, message]) => {
			if (!server) {
				return;
			}

			this.angulartics.eventTrack.next({
				action: 'ConnectFail',
				properties: {
					category: 'Server',
					label: server.address
				}
			});

			this.angulartics.eventTrack.next({
				action: 'ConnectFailMsg',
				properties: {
					category: 'Server',
					label: message
				}
			});
		});

		this.gameService.signinChange.subscribe((profile: Profile) => {
			if (profile.externalIdentifier.startsWith('dummy:')) {
				return;
			}
			
			this.angulartics.setUsername.next(profile.externalIdentifier);
		});
	}
}
