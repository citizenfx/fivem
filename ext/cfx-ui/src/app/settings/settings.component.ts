import { Component, OnInit, OnDestroy } from '@angular/core';
import { GameService } from '../game.service';
import { DiscourseService } from '../discourse.service';
import { ServersService } from '../servers/servers.service';
import { Language, TranslationService, Translation } from 'angular-l10n';
import { SettingsService, Setting } from '../settings.service';
import { Subscription } from 'rxjs';

class SelectOption {
    name: string;
    value: string;
}

class DisplaySetting {
    show: boolean;
    label: string;
    optionsArray: SelectOption[] = [];

    private _value: string;
    private subscriptions: Subscription[] = [];

    constructor(public setting: Setting) {
        this.show = (setting.showCb) ? false : true;
        this.label = '';
        this._value = '';

        if (setting.getCb) {
            this.subscriptions.push(setting.getCb().subscribe(value => this._value = value));
        }

        if (setting.showCb) {
            this.subscriptions.push(setting.showCb().subscribe(value => this.show = value));
        }

        if (setting.labelCb) {
            this.subscriptions.push(setting.labelCb().subscribe(value => this.label = value));
        }

        if (setting.options) {
            this.optionsArray = Object.entries(setting.options).map(([ value, name ]) => ({ name, value }));
        }
    }

    public get value(): string {
        return this._value;
    }

    public set value(value: string) {
        if (this.setting.setCb) {
            this.setting.setCb(value);
        }
    }

    public get boolValue(): boolean {
        return this.value === 'true';
    }

    public set boolValue(value: boolean) {
        this.value = value ? 'true' : 'false';
    }

    unsubscribe() {
        for (const subscription of this.subscriptions) {
            subscription.unsubscribe();
        }

        this.subscriptions = [];
    }
}

@Component({
    moduleId: module.id,
    selector: 'app-settings',
    templateUrl: 'settings.component.html',
	styleUrls:   ['settings.component.scss']
})

export class SettingsComponent extends Translation implements OnInit, OnDestroy {
    nickname = '';
    localhostPort = '30120';
    devMode = false;
    currentAccount: any = null;
    darkTheme = false;
    language = 'en';

    public settings: DisplaySetting[] = [];

    @Language() lang: string;

    constructor(private gameService: GameService, private discourseService: DiscourseService,
        private serversService: ServersService,
        public translation: TranslationService, private settingsService: SettingsService) {
        super();

        gameService.nicknameChange.subscribe(value => this.nickname = value);
        gameService.devModeChange.subscribe(value => this.devMode = value);
        gameService.localhostPortChange.subscribe(value => this.localhostPort = value);
        gameService.darkThemeChange.subscribe(value => this.darkTheme = value);
        gameService.languageChange.subscribe(value => this.language = value);

        discourseService.signinChange.subscribe(user => this.currentAccount = user);
    }

    registerConvar(name: string, valueCallback: (value: string) => void) {
        this.gameService.getConvar(name).subscribe(value => valueCallback(value));
    }

    ngOnInit() {
        this.nickname = this.gameService.nickname;
        this.devMode = this.gameService.devMode;
        this.localhostPort = this.gameService.localhostPort;
        this.darkTheme = this.gameService.darkTheme;
        this.language = this.gameService.language;
        this.currentAccount = this.discourseService.currentUser;

        for (const setting of this.settingsService.settingsList) {
            this.settings.push(new DisplaySetting(setting));
        }
    }

    ngOnDestroy() {
        for (const setting of this.settings) {
            setting.unsubscribe();
        }
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

    languageChanged(lang) {
        this.gameService.language = lang;
    }

    toggleConvar(name: string) {
        this.gameService.setConvar(name, this.gameService.getConvarValue(name) === 'true' ? 'false' : 'true');
    }
}
