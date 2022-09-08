import emojiList from 'emoji.json/emoji-compact.json';
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Avatar } from "cfx/ui/Avatar/Avatar";
import { ServerListItem } from "cfx/common/parts/Server/ServerListItem/ServerListItem";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { BrandIcon, Icons } from "cfx/ui/Icons";
import { BsDisplay } from "react-icons/bs";
import { ISetting, ISettings } from "cfx/common/services/settings/types";
import { useService } from "cfx/base/servicesContainer";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { CurrentGameName } from "cfx/base/gameName";
import { CustomBackdropControl } from "./components/CustomBackdropControl/CustomBackdropControl";
import { LinkedIdentitiesList } from "./parts/LinkedIdentitiesList/LinkedIdentitiesList";
import { mpMenu } from "./mpMenu";
import { IConvarService, KnownConvars, useConvarService } from "./services/convars/convars.service";
import { IConvar } from "./services/convars/types";
import { Input } from "cfx/ui/Input/Input";
import { Button } from "cfx/ui/Button/Button";
import { useAccountService } from "cfx/common/services/account/account.service";
import { ILinkedIdentitiesService } from "./services/linkedIdentities/linkedIdentities.service";
import { IServersBoostService } from 'cfx/common/services/servers/serversBoost.service';
import { Box } from 'cfx/ui/Layout/Box/Box';
import { GameName } from 'cfx/base/game';
import { useAuthService } from './services/auth/auth.service';
import { useIntlService } from 'cfx/common/services/intl/intl.service';
import { $L } from 'cfx/common/services/intl/l10n';
import { Select } from 'cfx/ui/Select/Select';
import { DEFAULT_SERVER_PORT } from 'cfx/base/serverUtils';
import { Text } from 'cfx/ui/Text/Text';

const ACCOUNT_SETTINGS = new Map<string, ISetting.AnySetting>([
  ['nickname', {
    label: $L('#Settings_Nickname'),
    render: () => {
      const ConvarService = useService(IConvarService);

      return (
        <Input
          value={mpMenu.getPlayerNickname()}
          onChange={mpMenu.setPlayerNickname}
          placeholder={ConvarService.get('ui_extNickname')}
        />
      );
    },
  }],

  ['profiles', {
    label: $L('#Settings_ConnectedProfiles'),
    visible: () => {
      const LinkedIdentitiesService = useService(ILinkedIdentitiesService);

      return LinkedIdentitiesService.linkedIdentities.isResolved() && LinkedIdentitiesService.linkedIdentities.value.length > 0;
    },
    render: () => <LinkedIdentitiesList />,
  }],

  ['accountButton', {
    label: $L('#Settings_Account'),
    visible: () => !accountLoadedAndPresent(),
    render: () => {
      const AuthService = useAuthService();

      return (
        <Button
          text={$L('#Settings_AccountLink')}
          onClick={AuthService.openUI}
        />
      );
    },
  }],

  ['accountDisplayNode', {
    label: $L('#Settings_Account'),
    visible: accountLoadedAndPresent,
    render: () => {
      const ConvarService = useConvarService();
      const AccountService = useAccountService();

      if (ConvarService.getBoolean(KnownConvars.streamerMode)) {
        return (
          <span>
            {'<HIDDEN>'}
          </span>
        );
      }

      return (
        <Flex repell>
          <Flex centered>
            <Avatar
              url={AccountService.account?.getAvatarUrl() || ''}
            />

            <Text size="large">
              {AccountService.account?.username}
            </Text>
          </Flex>
        </Flex>
      );
    },
  }],

  ['boostLoading', {
    type: 'displayNode',

    label: $L('#Settings_Boost'),

    node: $L('#Settings_BoostLoading'),

    visible: () => !useService(IServersBoostService).currentBoostLoadComplete,
  }],
  ['boostNone', {
    type: 'displayNode',

    label: $L('#Settings_Boost'),

    node: $L('#Settings_BoostNone'),

    visible: () => !boostLoadedAndPresent(),
  }],
  ['boostServer', {
    label: $L('#Settings_Boost'),
    visible: () => boostLoadedAndPresent(),
    render: () => {
      const { address } = useService(IServersBoostService).currentBoost!;

      const server = useService(IServersService).getServer(address);
      if (!server) {
        return (
          <Indicator />
        );
      }

      return (
        <Box height={10}>
          <ServerListItem
            server={server}
            standalone
            hideActions
            descriptionUnderName
          />
        </Box>
      );
    },

  }],
]);

const INTERFACE_SETTINGS = new Map<string, ISetting.AnySetting>([
  ['darkTheme', {
    type: 'checkbox',

    label: $L('#Settings_DarkTheme'),
    description: $L('#Settings_DarkThemeDesc'),

    ...convarAccessorsBoolean('ui_preferLightColorScheme', true),

    visible: onlyForFiveM,
  }],

  ['streamerMode', {
    type: 'checkbox',

    label: $L('#Settings_StreamerMode'),
    description: $L('#Settings_StreamerModeDesc'),

    ...convarAccessorsBoolean(KnownConvars.streamerMode),
  }],

  ['localhostPort', {
    type: 'text',
    inputType: 'number',

    label: $L('#Settings_LocalhostPort2'),
    description: $L('#Settings_LocalhostPort2'),
    placeholder: '30120',

    ...convarAccessorsString(KnownConvars.localhostPort, DEFAULT_SERVER_PORT),
  }],

  ['language', {
    type: 'displayNode',

    label: $L('#Settings_Language'),

    node() {
      const IntlService = useIntlService();

      return (
        <Select
          value={IntlService.localeCode}
          options={IntlService.localesOptions}
          onChange={IntlService.setLocale}
        />
      );
    },
  }],

  ['menuAudio', {
    type: 'checkbox',

    label: $L('#Settings_MenuAudio'),
    description: $L('#Settings_MenuAudioDesc'),

    ...convarAccessorsBoolean('ui_disableMusicTheme', true),
  }],

  ['customBackdropButton', {
    type: 'displayNode',

    label: $L('#Settings_CustomBackdrop'),
    description: $L('#Settings_CustomBackdropSelect'),

    node: () => <CustomBackdropControl />,
  }],
]);

const GAME_GAME_SETTINGS = new Map<string, ISetting.AnySetting>([
  ['updateChannel', {
    type: 'switch',

    label: $L('#Settings_UpdateChannel'),
    description: $L('#Settings_UpdateChannelDesc'),

    ...convarAccessorsString('ui_updateChannel'),

    options: {
      production: 'Release',
      beta: 'Beta',
      canary: 'Latest (Unstable)',
    },
  }],

  ['inProcessGPU', {
    type: 'checkbox',

    label: $L('#Settings_InProcessGpu'),
    description: $L('#Settings_InProcessGpuDesc'),

    ...convarAccessorsBoolean('nui_useInProcessGpu'),

    visible: onlyForFiveM,
  }],
  ['streamingProgress', {
    type: 'checkbox',

    label: $L('#Settings_GameStreamProgress'),
    description: $L('#Settings_GameStreamProgressDesc'),

    ...convarAccessorsBoolean('game_showStreamingProgress'),

    visible: onlyForFiveM,
  }],
  ['useAudioFrameLimiter', {
    type: 'checkbox',

    label: $L('#Settings_UseAudioFrameLimiter'),
    description: $L('#Settings_UseAudioFrameLimiterDesc'),

    ...convarAccessorsBoolean('game_useAudioFrameLimiter', true),

    visible: onlyForFiveM,
  }],
  ['enableHandbrakeCamera', {
    type: 'checkbox',

    label: $L('#Settings_HandbrakeCamera'),
    description: $L('#Settings_HandbrakeCameraDesc'),

    ...convarAccessorsBoolean('cam_enableHandbrakeCamera'),

    visible: onlyForFiveM,
  }],
  ['disableCameraShake', {
    type: 'checkbox',

    label: $L('#Settings_CameraShake'),
    description: $L('#Settings_CameraShakeDesc'),

    ...convarAccessorsBoolean('cam_disableCameraShake'),

    visible: onlyForFiveM,
  }],

  ['customEmoji', {
    type: 'switch',

    multiline: true,

    label: $L('#Settings_CustomEmoji'),
    description: $L('#Settings_CustomEmojiDesc'),

    ...convarAccessorsString('ui_customBrandingEmoji'),

    options: Object.fromEntries([
      ['', 'Default'],
      ...emojiList.filter((emoji) => emoji.length === 2).map((emoji) => [emoji, emoji]),
    ]),

    visible: () => useService(IConvarService).getBoolean('ui_premium'),
  }],
  ['customEmojiUpsell', {
    type: 'displayNode',

    label: $L('#Settings_CustomEmoji'),

    node: $L('#Settings_CustomEmojiUpsell'),

    visible: () => !useService(IConvarService).getBoolean('ui_premium'),
  }],
]);

export const GAME_SETTINGS: ISettings = new Map([
  ['account', {
    icon: Icons.account,
    label: $L('#SettingsCat_Account'),

    settings: ACCOUNT_SETTINGS,
  }],

  ['interface', {
    icon: <BsDisplay />,
    label: $L('#SettingsCat_Interface'),

    settings: INTERFACE_SETTINGS,
  }],

  ['game', {
    icon: BrandIcon[CurrentGameName],
    label: $L('#SettingsCat_Game'),

    settings: GAME_GAME_SETTINGS,
  }],
]);

export const DEFAULT_GAME_SETTINGS_CATEGORY_ID = [...GAME_SETTINGS.keys()][0];

function onlyForFiveM() {
  return CurrentGameName === GameName.FiveM;
}

function convarAccessorsBoolean(convar: IConvar, inversion = false): Pick<ISetting.Checkbox, 'accessors'> {
  if (inversion) {
    return {
      accessors: () => {
        const ConvarService = useService(IConvarService);

        return {
          getValue: () => !ConvarService.getBoolean(convar),
          setValue: (value) => ConvarService.setBoolean(convar, !value),
        };
      },
    };
  }

  return {
    accessors: () => {
      const ConvarService = useService(IConvarService);

      return {
        getValue: () => ConvarService.getBoolean(convar),
        setValue: (value) => ConvarService.setBoolean(convar, value),
      };
    },
  };
}

function convarAccessorsString(convar: IConvar, defaultValue = ''): Pick<ISetting.Text, 'accessors'> {
  return {
    accessors: () => {
      const ConvarService = useService(IConvarService);

      return {
        getValue: () => ConvarService.get(convar) || defaultValue,
        setValue: (value) => ConvarService.set(convar, value),
      };
    },
  };
}

function accountLoadedAndPresent(): boolean {
  const AccountService = useAccountService();

  return AccountService.accountLoadComplete && !!AccountService.account;
}

function boostLoadedAndPresent(): boolean {
  const ServersBoostService = useService(IServersBoostService);

  return ServersBoostService.currentBoostLoadComplete && !!ServersBoostService.currentBoost;
}
