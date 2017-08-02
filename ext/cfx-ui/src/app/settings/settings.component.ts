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

    constructor(private gameService: GameService) { }

    ngOnInit() {
        this.nickname = this.gameService.nickname;
    }

    nameChanged(newName) {
        this.gameService.nickname = newName;
    }
}