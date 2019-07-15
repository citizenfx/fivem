import { Component, Input, ViewChild, ChangeDetectionStrategy } from '@angular/core';
import { Router } from '@angular/router';

import { Server } from '../server';

import { GameService } from '../../game.service'
import { DiscourseService, BoostData } from 'app/discourse.service';

@Component({
    moduleId: module.id,
    selector: 'servers-list-item',
    templateUrl: 'servers-list-item.component.html',
    styleUrls: ['servers-list-item.component.scss'],
    changeDetection: ChangeDetectionStrategy.OnPush
})

export class ServersListItemComponent {
    @Input()
    server: Server;

    @Input()
    pinned = false;

    private upvoting = false;

    constructor(private gameService: GameService, private discourseService: DiscourseService,
        private router: Router) { }

    attemptConnect(event: Event) {
        this.gameService.connectTo(this.server);

        event.stopPropagation();
    }

    showServerDetail() {
        this.router.navigate(['/', 'servers', 'detail', this.server.address]);
    }

    isFavorite() {
        return this.gameService.isMatchingServer('favorites', this.server);
    }

    toggleFavorite(event: Event) {
        if (this.isFavorite()) {
            this.removeFavorite();
        } else {
            this.addFavorite();
        }

        event.stopPropagation();
    }

    addFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, true);
    }

    removeFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, false);
    }

    enableBoost(event: Event) {
        if (!this.isBoost()) {
            this.addBoost();
        }

        event.stopPropagation();
    }

    isBoost() {
        return this.discourseService.currentBoost && this.discourseService.currentBoost.address === this.server.address;
    }

    addBoost() {
        if (!this.discourseService.currentUser) {
            this.gameService.invokeInformational(
                'You need to have a linked FiveM account with an active Patreon subscription in order to BOOST™ a server.'
            );

            return;
        }

        if (!this.discourseService.currentUser.isPremium) {
            this.gameService.invokeInformational(
                'You need to have an active Patreon "Element Club" subscription in order to BOOST™ a server. ' +
                'Please get one to be able to support this server!'
            );

            return;
        }

        if (this.isBoost()) {
            return;
        }

        if (this.upvoting) {
            return;
        }

        this.discourseService.externalCall('https://servers-frontend.fivem.net/api/upvote/', 'POST', {
            address: this.server.address
        }).then((response) => {
            if (response.data) {
                if (!this.discourseService.currentBoost) {
                    this.discourseService.currentBoost = new BoostData();
                }

                this.discourseService.currentBoost.address = this.server.address;
                this.discourseService.currentBoost.server = this.server;

                this.gameService.invokeInformational(
                    'Your BOOST™ is now assigned to this server! Thanks for helping the server go higher.'
                );
            } else {
                this.gameService.invokeError(
                    ':( Assigning BOOST™ failed. Please try again later, or contact FiveM support if this issue persists!'
                );
            }

            this.upvoting = false;
        });
    }
}
