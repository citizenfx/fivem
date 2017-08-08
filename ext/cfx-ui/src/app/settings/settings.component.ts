import { Component, OnInit } from '@angular/core';
import { GameService } from '../game.service';

@Component({
    moduleId: module.id,
    selector: 'app-settings',
    templateUrl: 'settings.component.html',
	styleUrls:   ['settings.component.scss']
})

export class SettingsComponent implements OnInit {
    nickname = '';
    devMode = false;

    constructor(private gameService: GameService) {
        gameService.nicknameChange.subscribe(value => this.nickname = value);
        gameService.devModeChange.subscribe(value => this.devMode = value);
    }

    ngOnInit() {
        this.nickname = this.gameService.nickname;
        this.devMode = this.gameService.devMode;
    }

    nameChanged(newName) {
        this.gameService.nickname = newName;
    }

    toggleDevMode() {
        this.gameService.devMode = !this.devMode;
    }
}