import { Component, OnInit, OnDestroy, Inject } from '@angular/core';
import { GameService } from '../game.service';
import { DiscourseService } from '../discourse.service';
import { ServersService } from '../servers/servers.service';
import { L10nTranslationService, L10nLocale, L10N_LOCALE } from 'angular-l10n';
import { SettingsService, Setting } from '../settings.service';
import { Subscription } from 'rxjs';

class SelectOption {
    name: string;
    value: string;
}

class DisplaySetting {
    show: boolean;
    label: string;
    category: string;
    optionsArray: SelectOption[] = [];

    private _value: string;
    private subscriptions: Subscription[] = [];

    constructor(public setting: Setting) {
        this.show = (setting.showCb) ? false : true;
        this.category = setting.category;
        this.label = '';
        this._value = (setting.displayDefault) ? setting.displayDefault : '';

        if (setting.getCb) {
            this.subscriptions.push(setting.getCb().subscribe(value => this._value = value));
        }

        if (setting.showCb) {
            this.subscriptions.push(setting.showCb().subscribe(value => {
				this.show = value;
			}));
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

export class SettingsComponent implements OnInit, OnDestroy {
    nickname = '';
    localhostPort = '30120';
    devMode = false;
    currentAccount: any = null;
    darkTheme = true;
    language = 'en';

    public categories: string[] = [];
    public selectedCategory: string;

	public settings: DisplaySetting[] = [];

	categorizedSettings: Map<string, DisplaySetting[]> = new Map();

    constructor(private gameService: GameService, private discourseService: DiscourseService,
        private serversService: ServersService, @Inject(L10N_LOCALE) public locale: L10nLocale,
        public translation: L10nTranslationService, private settingsService: SettingsService) {
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
			const displaySetting = new DisplaySetting(setting);
			let settingCategory = [];

			if (!this.categorizedSettings.has(displaySetting.category)) {
				this.categorizedSettings.set(displaySetting.category, settingCategory);
			} else {
				settingCategory = this.categorizedSettings.get(displaySetting.category);
			}

            settingCategory.push(displaySetting);
        }

        this.categories = Array.from(new Set<string>(this.settings.map(a => a.category)).values());
        this.selectedCategory = this.categories[0];
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

    setCategory(category: string) {
        this.selectedCategory = category;
    }

    shouldShowCategory(category: string): boolean {
        return !!this.settings.find(a => a.show && a.category === category);
    }
}
