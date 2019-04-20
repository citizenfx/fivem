import { Component, OnInit } from '@angular/core';
import { GameService } from '../game.service';
import { DiscourseService, BoostData } from '../discourse.service';
import { ServersService } from '../servers/servers.service';
import { Http } from '@angular/http';

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
    darkTheme = false;
    currentBoost: BoostData = null;
    noCurrentBoost = false;

    constructor(private gameService: GameService, private discourseService: DiscourseService,
        private serversService: ServersService, private http: Http) {
        gameService.nicknameChange.subscribe(value => this.nickname = value);
        gameService.devModeChange.subscribe(value => this.devMode = value);
        gameService.localhostPortChange.subscribe(value => this.localhostPort = value);
        gameService.darkThemeChange.subscribe(value => this.darkTheme = value);

        discourseService.signinChange.subscribe(user => this.currentAccount = user);
    }

    ngOnInit() {
        this.nickname = this.gameService.nickname;
        this.devMode = this.gameService.devMode;
        this.localhostPort = this.gameService.localhostPort;
        this.darkTheme = this.gameService.darkTheme;
        this.currentAccount = this.discourseService.currentUser;
        this.currentBoost = this.discourseService.currentBoost;
        this.noCurrentBoost = this.discourseService.noCurrentBoost;
    }

    nameChanged(newName) {
        this.gameService.nickname = newName;
    }

    toggleDevMode() {
        this.gameService.devMode = !this.devMode;
    }

    toggleDarkTheme() {
        this.gameService.darkTheme = !this.darkTheme;
    }

    localhostPortChanged(newPort) {
        this.gameService.localhostPort = newPort;
    }

    async linkAccount() {
        const url = await this.discourseService.generateAuthURL();
        this.gameService.openUrl(url);
    }
}
