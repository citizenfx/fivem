import { Injectable } from '@angular/core';
import { Observable, combineLatest, of } from 'rxjs';
import { map } from 'rxjs/operators';

import { TranslationService } from 'angular-l10n';
import { GameService } from './game.service';

import { Languages } from './languages';
import { DiscourseService } from './discourse.service';

import * as emojiList from 'emoji.json/emoji-compact.json';

export class Setting {
    name: string;
    description?: string;
    type: 'checkbox' | 'text' | 'select' | 'label' | 'button';
    options?: { [value: string]: string };
    showCb?: () => Observable<boolean>;
    labelCb?: () => Observable<string>;
    colorizeValue?: boolean;

    getCb?: () => Observable<string>;
    setCb?: (value: string) => void;

    id?: string;
}

function fromEntries<TValue>(iterable: [string, TValue][]): { [key: string]: TValue } {
    return [...iterable].reduce<{ [key: string]: TValue }>((obj, [key, val]) => {
        (obj as any)[key] = val;
        return obj;
    }, {} as any);
}

@Injectable()
export class SettingsService {
    private settings: { [key: string]: Setting } = {};
    private settingOrder: string[] = [];

    constructor(private translation: TranslationService, private gameService: GameService,
        private discourseService: DiscourseService) {
        this.addSetting('nickname', {
            name: '#Settings_Nickname',
            description: '#Settings_Nickname',
            type: 'text',
            getCb: () => this.gameService.nicknameChange,
            setCb: (value) => this.gameService.nickname = value,
        });

        if (this.gameService.gameName !== 'rdr3') {
            this.addSetting('darkTheme', {
                name: '#Settings_DarkTheme',
                description: '#Settings_DarkThemeDesc',
                type: 'checkbox',
                getCb: () => this.gameService.darkThemeChange.map(a => a ? 'true' : 'false'),
                setCb: (value) => this.gameService.darkTheme = (value === 'true')
            });
        }

        this.addSetting('devMode', {
            name: '#Settings_DevMode',
            description: '#Settings_DevModeDesc',
            type: 'checkbox',
            getCb: () => this.gameService.devModeChange.map(a => a ? 'true' : 'false'),
            setCb: (value) => this.gameService.devMode = (value === 'true')
        });

        this.addSetting('localhostPort', {
            name: '#Settings_LocalhostPort',
            description: '#Settings_LocalhostPort',
            type: 'text',
            getCb: () => this.gameService.localhostPortChange,
            setCb: (value) => this.gameService.localhostPort = value,
            showCb: () => this.gameService.devModeChange
        });

        this.addSetting('language', {
            name: '#Settings_Language',
            type: 'select',
            getCb: () => this.gameService.languageChange,
            setCb: (value) => this.gameService.language = value,
            options: Languages.toSettingsOptions()
        });

        if (this.gameService.gameName !== 'rdr3') {
            this.addSetting('menuAudio', {
                name: '#Settings_MenuAudio',
                description: '#Settings_MenuAudioDesc',
                type: 'checkbox',
                getCb: () => this.gameService.getConvar('ui_disableMusicTheme').pipe(map(a => a === 'true' ? 'false' : 'true')),
                setCb: (value) => this.gameService.setConvar('ui_disableMusicTheme', value === 'true' ? 'false' : 'true')
            });

            this.addSetting('streamingProgress', {
                name: '#Settings_GameStreamProgress',
                description: '#Settings_GameStreamProgressDesc',
                type: 'checkbox',
                getCb: () => this.gameService.getConvar('game_showStreamingProgress'),
                setCb: (value) => this.gameService.setConvar('game_showStreamingProgress', value)
            });

            this.addSetting('customEmoji', {
                name: '#Settings_CustomEmoji',
                description: '#Settings_CustomEmojiDesc',
                type: 'select',
                getCb: () => this.gameService.getConvar('ui_customBrandingEmoji'),
                setCb: (value) => this.gameService.setConvar('ui_customBrandingEmoji', value),
                showCb: () => this.gameService.getConvar('ui_premium').pipe(map(a => a === 'true')),
                options: fromEntries([
                    [ '', 'Default' ],
                    ...emojiList.default.filter(emoji => emoji.length === 2).map(emoji => [ emoji, emoji ])
                ])
            });

            this.addSetting('customEmojiUpsell', {
                name: '#Settings_CustomEmoji',
                type: 'label',
                showCb: () => this.gameService.getConvar('ui_premium').pipe(map(a => a !== 'true')),
                labelCb: () => translation.translationChanged().pipe().map(_ => translation.translate('#Settings_CustomEmojiUpsell'))
            });
        }

        this.addSetting('queriesPerMinute', {
            name: '#Settings_QueriesPerMinute',
            type: 'select',
            getCb: () => this.gameService.getConvar('ui_maxQueriesPerMinute'),
            setCb: (value) => this.gameService.setConvar('ui_maxQueriesPerMinute', value),
            options: {
                '10000': '10000',
                '5000': '5000',
                '3000': '3000',
                '1500': '1500',
                '1000': '1000',
                '500': '500',
                '250': '250',
            }
        });

        if (this.gameService.gameName !== 'rdr3') {
            this.addSetting('accountButton', {
                name: '#Settings_Account',
                type: 'button',
                description: '#Settings_AccountLink',
                setCb: (value) => this.linkAccount(),
                showCb: () => discourseService.signinChange.pipe(map(user => !user)),
            });

            this.addSetting('accountLabel', {
                name: '#Settings_Account',
                type: 'label',
                showCb: () => discourseService.signinChange.pipe(map(user => !!user)),
                labelCb: () =>
                    combineLatest(
                        discourseService.signinChange,
                        translation.translationChanged()
                    ).pipe(map(
                        ([ user, _ ]) => translation.translate('#Settings_AccountLinked', { username: user ? user.username : '' })
                    ))
            });

            this.addSetting('boostLoading', {
                name: '#Settings_Boost',
                type: 'label',
                showCb: () => discourseService.signinChange.pipe(map(user => !!user && !this.discourseService.currentBoost
                    && !this.discourseService.noCurrentBoost)),
                labelCb: () => translation.translationChanged().pipe().map(_ => translation.translate('#Settings_BoostLoading'))
            });

            this.addSetting('boostNone', {
                name: '#Settings_Boost',
                type: 'label',
                showCb: () => discourseService.signinChange.pipe(map(user => !!user && !this.discourseService.currentBoost
                    && this.discourseService.noCurrentBoost)),
                labelCb: () => translation.translationChanged().pipe().map(_ => translation.translate('#Settings_BoostNone'))
            });

            this.addSetting('boostUnknown', {
                name: '#Settings_Boost',
                type: 'label',
                showCb: () => discourseService.signinChange.pipe(map(user => !!user && this.discourseService.currentBoost
                    && !this.discourseService.currentBoost.server)),
                labelCb: () => of(this.discourseService.currentBoost ? this.discourseService.currentBoost.address : '')
            });

            this.addSetting('boostServer', {
                name: '#Settings_Boost',
                type: 'label',
                showCb: () => discourseService.signinChange.pipe(map(user => !!user && this.discourseService.currentBoost
                    && !!this.discourseService.currentBoost.server)),
                labelCb: () => of(
                    this.discourseService.currentBoost.server && this.discourseService.currentBoost.server.hostname ?
                        this.discourseService.currentBoost.server.hostname : 'test'
                ),
                colorizeValue: true
            });
        }
    }

    public addSetting(label: string, setting: Setting) {
        setting.id = label;

        if (!setting.description) {
            setting.description = '';
        }

        this.settings[label] = setting;
        this.settingOrder.push(label);
    }

    public get settingsList() {
        return this.settingOrder.map(key => this.settings[key]);
    }

    private async linkAccount() {
        const url = await this.discourseService.generateAuthURL();
        this.gameService.openUrl(url);
    }
}
