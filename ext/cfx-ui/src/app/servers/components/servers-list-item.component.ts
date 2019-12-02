import { Component, Input, ViewChild, ChangeDetectionStrategy, OnDestroy, OnInit, ElementRef, OnChanges, AfterViewInit } from '@angular/core';
import { Router } from '@angular/router';

import { Server } from '../server';

import { GameService } from '../../game.service'
import { DiscourseService, BoostData } from 'app/discourse.service';

import * as hoverintent from 'hoverintent';
import { ServersService } from '../servers.service';

@Component({
    moduleId: module.id,
    selector: 'servers-list-item',
    templateUrl: 'servers-list-item.component.html',
    styleUrls: ['servers-list-item.component.scss'],
    changeDetection: ChangeDetectionStrategy.OnPush
})

export class ServersListItemComponent implements OnInit, OnDestroy {
    @Input()
    server: Server;

    @Input()
    pinned = false;

    private hoverIntent: any;

    private upvoting = false;

    constructor(private gameService: GameService, private discourseService: DiscourseService,
        private serversService: ServersService, private router: Router, private elementRef: ElementRef) { }

    public ngOnInit() {
        this.hoverIntent = hoverintent(this.elementRef.nativeElement, () => {
            this.serversService.getServer(this.server.address, true);
        }, () => {});

        this.hoverIntent.options({
            interval: 50
        });
    }

    public ngOnDestroy() {
        this.hoverIntent.remove();
    }

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
                'You need to have a linked FiveM account in order to BOOST™ a server.'
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
            if (response.data.success) {
                if (!this.discourseService.currentBoost) {
                    this.discourseService.currentBoost = new BoostData();
                }

                this.discourseService.currentBoost.address = this.server.address;
                this.discourseService.currentBoost.server = this.server;

                this.gameService.invokeInformational(
                    `Your BOOST™ is now assigned to this server (with an admirable strength of ${response.data.power})! `
                    + 'Thanks for helping the server go higher.'
                );
            } else if (response.data.error) {
                this.gameService.invokeError(
                    response.data.error
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
