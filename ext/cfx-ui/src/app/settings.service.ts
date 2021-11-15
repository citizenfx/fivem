import { Injectable } from '@angular/core';
import { Observable, combineLatest, of, BehaviorSubject } from 'rxjs';
import { map } from 'rxjs/operators';

import { L10nTranslationService } from 'angular-l10n';
import { GameService } from './game.service';

import { Languages } from './languages';
import { DiscourseService } from './discourse.service';

import * as emojiList from 'emoji.json/emoji-compact.json';

export class Setting {
	name: string;
	description?: string;
	type: 'checkbox' | 'text' | 'select' | 'label' | 'button' | 'html' | 'switch';
	options?: { [value: string]: string };
	showCb?: () => Observable<boolean>;
	labelCb?: () => Observable<string>;
	colorizeValue?: boolean;
	category?: string;
	displayDefault?: string;

	getCb?: () => Observable<string>;
	setCb?: (value: string) => void;

	id?: string;
}

export function fromEntries<TValue>(iterable: [string, TValue][]): { [key: string]: TValue } {
	return [...iterable].reduce<{ [key: string]: TValue }>((obj, [key, val]) => {
		(obj as any)[key] = val;
		return obj;
	}, {} as any);
}

@Injectable()
export class SettingsService {
	private settings: { [key: string]: Setting } = {};
	private settingOrder: string[] = [];

	constructor(
        private translation: L10nTranslationService,
        private gameService: GameService,
		private discourseService: DiscourseService,
    ) {
		this.addSetting('nickname', {
			name: '#Settings_Nickname',
			description: '#Settings_Nickname',
			type: 'text',
			getCb: () => this.gameService.nicknameChange,
			setCb: (value) => this.gameService.nickname = value,
			category: '#SettingsCat_Connection',
		});

		if (this.gameService.gameName !== 'rdr3' && this.gameService.gameName !== 'ny') {
			this.addSetting('darkTheme', {
				name: '#Settings_DarkTheme',
				description: '#Settings_DarkThemeDesc',
				type: 'checkbox',
				getCb: () => this.gameService.darkThemeChange.map(a => a ? 'true' : 'false'),
				setCb: (value) => this.gameService.darkTheme = (value === 'true'),
				category: '#SettingsCat_Interface',
			});
		}

		this.addSetting('streamerMode', {
			name: '#Settings_StreamerMode',
			description: '#Settings_StreamerModeDesc',
			type: 'checkbox',
			getCb: () => this.gameService.streamerModeChange.map(a => a ? 'true' : 'false'),
			setCb: (value) => this.gameService.streamerMode = (value === 'true'),
			category: '#SettingsCat_Interface',
		});

        this.addSetting('customBackdropButton', {
            name: '#Settings_CustomBackdrop',
            type: 'button',
            setCb: (value) => this.setCustomBackdrop(),
            category: '#SettingsCat_Interface',
            description: '#Settings_CustomBackdropSelect',
        });

		this.addSetting('uiPerformance', {
			name: '#Settings_LowPerfMode',
			description: '#Settings_LowPerfModeDesc',
			type: 'switch',
			getCb: () => this.gameService.getConvar('ui_blurPerfMode'),
			setCb: (value) => this.gameService.setArchivedConvar('ui_blurPerfMode', value),
			options: {
				'off': '#Settings_LowPerf_Off',
				'backdrop': '#Settings_LowPerf_ReduceBackdrop',
				'none': '#Settings_LowPerf_ReduceFull',
			},
			displayDefault: 'off',
			category: '#SettingsCat_Interface'
		})

		this.addSetting('localhostPort', {
			name: '#Settings_LocalhostPort2',
			description: '#Settings_LocalhostPort2',
			type: 'text',
			getCb: () => this.gameService.localhostPortChange,
			setCb: (value) => this.gameService.localhostPort = value,
			category: '#SettingsCat_Interface',
		});

		this.addSetting('language', {
			name: '#Settings_Language',
			type: 'select',
			getCb: () => this.gameService.languageChange,
			setCb: (value) => this.gameService.language = value,
			options: Languages.toSettingsOptions(),
			category: '#SettingsCat_Interface',
		});

		if (this.gameService.gameName !== 'rdr3' && this.gameService.gameName !== 'ny') {
			this.addSetting('menuAudio', {
				name: '#Settings_MenuAudio',
				description: '#Settings_MenuAudioDesc',
				type: 'checkbox',
				getCb: () => this.gameService.getConvar('ui_disableMusicTheme').pipe(map(a => a === 'true' ? 'false' : 'true')),
				setCb: (value) => this.gameService.setConvar('ui_disableMusicTheme', value === 'true' ? 'false' : 'true'),
				category: '#SettingsCat_Interface',
			});

			this.addSetting('inProcessGpu', {
				name: '#Settings_InProcessGpu',
				description: '#Settings_InProcessGpuDesc',
				type: 'checkbox',
				getCb: () => this.gameService.getConvar('nui_useInProcessGpu').pipe(map(a => a === 'true' ? 'true' : 'false')),
				setCb: (value) => this.gameService.setConvar('nui_useInProcessGpu', value),
				category: '#SettingsCat_Game',
			});

			this.addSetting('streamingProgress', {
				name: '#Settings_GameStreamProgress',
				description: '#Settings_GameStreamProgressDesc',
				type: 'checkbox',
				getCb: () => this.gameService.getConvar('game_showStreamingProgress').pipe(map(a => a === 'true' ? 'true' : 'false')),
				setCb: (value) => this.gameService.setConvar('game_showStreamingProgress', value),
				category: '#SettingsCat_Game',
			});

			this.addSetting('useAudioFrameLimiter', {
				name: '#Settings_UseAudioFrameLimiter',
				description: '#Settings_UseAudioFrameLimiterDesc',
				type: 'checkbox',
				getCb: () => this.gameService.getConvar('game_useAudioFrameLimiter').pipe(map(a => a === 'true' ? 'false' : 'true')),
				setCb: (value) => this.gameService.setConvar('game_useAudioFrameLimiter', value === 'true' ? 'false' : 'true'),
				category: '#SettingsCat_Game',
			});

			this.addSetting('enableHandbrakeCamera', {
				name: '#Settings_HandbrakeCamera',
				description: '#Settings_HandbrakeCameraDesc',
				type: 'checkbox',
				getCb: () => this.gameService.getConvar('cam_enableHandbrakeCamera').pipe(map(a => a === 'true' ? 'true' : 'false')),
				setCb: (value) => this.gameService.setConvar('cam_enableHandbrakeCamera', value),
				category: '#SettingsCat_Game',
			});

			this.addSetting('disableCameraShake', {
				name: '#Settings_CameraShake',
				description: '#Settings_CameraShakeDesc',
				type: 'checkbox',
				getCb: () => this.gameService.getConvar('cam_disableCameraShake').pipe(map(a => a === 'true' ? 'true' : 'false')),
				setCb: (value) => this.gameService.setConvar('cam_disableCameraShake', value),
				category: '#SettingsCat_Game',
			});

			this.addSetting('mumbleNoiseSuppression', {
				name: '#Settings_MumbleNoiseSuppression',
				description: '#Settings_MumbleNoiseSuppressionDesc',
				type: 'checkbox',
				getCb: () => this.gameService.getConvar('mumble_enableNoiseSuppression').pipe(map(a => a === 'true' ? 'true' : 'false')),
				setCb: (value) => this.gameService.setConvar('mumble_enableNoiseSuppression', value),
				category: '#SettingsCat_Game',
			});

			this.addSetting('customEmoji', {
				name: '#Settings_CustomEmoji',
				description: '#Settings_CustomEmojiDesc',
				type: 'select',
				getCb: () => this.gameService.getConvar('ui_customBrandingEmoji'),
				setCb: (value) => this.gameService.setConvar('ui_customBrandingEmoji', value),
				showCb: () => this.gameService.getConvar('ui_premium').pipe(map(a => a === 'true')),
				options: fromEntries([
					['', 'Default'],
					...emojiList.default.filter(emoji => emoji.length === 2).map(emoji => [emoji, emoji])
				]),
				category: '#SettingsCat_Game',
			});

			this.addSetting('customEmojiUpsell', {
				name: '#Settings_CustomEmoji',
				type: 'label',
				showCb: () => this.gameService.getConvar('ui_premium').pipe(map(a => a !== 'true')),
				labelCb: () => translation.onChange().pipe().map(_ => translation.translate('#Settings_CustomEmojiUpsell')),
				category: '#SettingsCat_Game',
			});
		}

		this.addSetting('connectedProfiles', {
			name: '#Settings_ConnectedProfiles',
			type: 'html',
			showCb: () => this.gameService.hasProfiles(),
			labelCb: () => this.gameService.getProfileString(),
			category: '#SettingsCat_Account',
		});

		if (this.gameService.gameName !== 'rdr3' && this.gameService.gameName !== 'ny') {
			this.addSetting('accountButton', {
				name: '#Settings_Account',
				type: 'button',
				description: '#Settings_AccountLink',
				setCb: (value) => this.linkAccount(),
				showCb: () => discourseService.signinChange.pipe(map(user => !user)),
				category: '#SettingsCat_Account',
			});

			this.addSetting('accountLabel', {
				name: '#Settings_Account',
				type: 'label',
				showCb: () => discourseService.signinChange.pipe(map(user => !!user)),
				labelCb: () =>
					combineLatest([
						gameService.streamerModeChange,
						discourseService.signinChange,
						translation.onChange()
					]).pipe(map(
						([streamerMode, user, _]) =>
							translation.translate('#Settings_AccountLinked', {
								username: streamerMode ? '<HIDDEN>' : (user?.username ?? '')
							})
					)),
				category: '#SettingsCat_Account',
			});

			this.addSetting('boostLoading', {
				name: '#Settings_Boost',
				type: 'label',
				showCb: () => discourseService.signinChange.pipe(map(user => !!user && !this.discourseService.currentBoost
					&& !this.discourseService.noCurrentBoost)),
				labelCb: () => translation.onChange().pipe().map(_ => translation.translate('#Settings_BoostLoading')),
				category: '#SettingsCat_Account',
			});

			this.addSetting('boostNone', {
				name: '#Settings_Boost',
				type: 'label',
				showCb: () => discourseService.signinChange.pipe(map(user => !!user && !this.discourseService.currentBoost
					&& this.discourseService.noCurrentBoost)),
				labelCb: () => translation.onChange().pipe().map(_ => translation.translate('#Settings_BoostNone')),
				category: '#SettingsCat_Account',
			});

			this.addSetting('boostUnknown', {
				name: '#Settings_Boost',
				type: 'label',
				showCb: () => discourseService.signinChange.pipe(map(user => !!user && this.discourseService.currentBoost
					&& !this.discourseService.currentBoost.server)),
				labelCb: () => of(this.discourseService.currentBoost ? this.discourseService.currentBoost.address : ''),
				category: '#SettingsCat_Account',
			});

			this.addSetting('boostServer', {
				name: '#Settings_Boost',
				type: 'label',
				showCb: () => discourseService.signinChange.pipe(map(user => !!user && this.discourseService.currentBoost
					&& !!this.discourseService.currentBoost.server)),
				labelCb: () => of(
					this.discourseService.currentBoost && this.discourseService.currentBoost.server &&
						this.discourseService.currentBoost.server.hostname ? this.discourseService.currentBoost.server.hostname : 'test'
				),
				colorizeValue: true,
				category: '#SettingsCat_Account',
			});
		}

		this.addSetting('updateChannel', {
			name: '#Settings_UpdateChannel',
			description: '#Settings_UpdateChannelDesc',
			type: 'switch',
			getCb: () => this.gameService.getConvar('ui_updateChannel'),
			setCb: (value) => this.gameService.setConvar('ui_updateChannel', value),
			options: {
				'production': 'Release',
				'canary': 'Canary (Experimental/Unstable)',
			},
			category: '#SettingsCat_Game',
		});
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
        this.discourseService.openAuthModal();
	}

    private async setCustomBackdrop() {
        const fileName = await this.gameService.selectFile('backdrop');
        this.gameService.setArchivedConvar('ui_customBackdrop', fileName);
    }
}
