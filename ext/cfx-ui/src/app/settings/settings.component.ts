import { Component, OnInit } from '@angular/core';
import { GameService } from '../game.service';
import { DiscourseService } from '../discourse.service';

@Component({
    moduleId: module.id,
    selector: 'app-settings',
    templateUrl: 'settings.component.html',
	styleUrls:   ['settings.component.scss']
})

export class SettingsComponent implements OnInit {
    nickname = '';
    localhostPort = '30120';
    devMode = false;
    currentAccount: any = null;

    constructor(private gameService: GameService, private discourseService: DiscourseService) {
        gameService.nicknameChange.subscribe(value => this.nickname = value);
        gameService.devModeChange.subscribe(value => this.devMode = value);
        gameService.localhostPortChange.subscribe(value => this.localhostPort = value);

        discourseService.signinChange.subscribe(user => this.currentAccount = user);
    }

    ngOnInit() {
        this.nickname = this.gameService.nickname;
        this.devMode = this.gameService.devMode;
        this.localhostPort = this.gameService.localhostPort;
        this.currentAccount = this.discourseService.currentUser;
    }

    nameChanged(newName) {
        this.gameService.nickname = newName;
    }

    toggleDevMode() {
        this.gameService.devMode = !this.devMode;
    }

    localhostPortChanged(newPort) {
        this.gameService.localhostPort = newPort;
    }

    async linkAccount() {
        const url = await this.discourseService.generateAuthURL();
        this.gameService.openUrl(url);
    }
}