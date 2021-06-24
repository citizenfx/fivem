export enum SystemResource {
  MAP_HIPSTER = 'map-hipster',
  MAP_SKATER = 'map-skater',

  BASIC_GAMEMODE = 'basic-gamemode',

  MONEY = 'money',
  MONEY_FOUNTAIN = 'money-fountain',
  MONEY_FOUNTAIN_EXAMPLE_MAP = 'money-fountain-example-map',
  PED_MONEY_DROPS = 'ped-money-drops',
  EXAMPLE_LOADSCREEN = 'example-loadscreen',

  CHAT = 'chat',
  CHAT_THEME_GTAO = 'chat-theme-gtao',

  PLAYER_DATA = 'player-data',
  PLAYER_NAMES = 'playernames',

  MAP_MANAGER = 'mapmanager',
  SPAWN_MANAGER = 'spawnmanager',

  WEBPACK = 'webpack',
  YARN = 'yarn',

  BASE_EVENTS = 'baseevents',
  HARD_CAP = 'hardcap',
  RCON_LOG = 'rconlog',
  RUNCODE = 'runcode',
  SESSION_MANAGER = 'sessionmanager',
};

export const SYSTEM_RESOURCES_URL = 'https://github.com/citizenfx/cfx-server-data.git';

export const SYSTEM_RESOURCES_MAPPING: Record<SystemResource, string> = Object.freeze({
  [SystemResource.MAP_HIPSTER]: '[gamemodes]/[maps]/fivem-map-hipster',
  [SystemResource.MAP_SKATER]: '[gamemodes]/[maps]/fivem-map-skater',

  [SystemResource.BASIC_GAMEMODE]: '[gamemodes]/basic-gamemode',

  [SystemResource.MONEY]: '[gameplay]/[examples]/money',
  [SystemResource.MONEY_FOUNTAIN]: '[gameplay]/[examples]/money-fountain',
  [SystemResource.MONEY_FOUNTAIN_EXAMPLE_MAP]: '[gameplay]/[examples]/money-fountain-example-map',
  [SystemResource.PED_MONEY_DROPS]: '[gameplay]/[examples]/ped-money-drops',
  [SystemResource.EXAMPLE_LOADSCREEN]: '[test]/example-loadscreen',

  [SystemResource.CHAT]: '[gameplay]/chat',
  [SystemResource.CHAT_THEME_GTAO]: '[gameplay]/chat-theme-gtao',

  [SystemResource.PLAYER_DATA]: '[gameplay]/player-data',
  [SystemResource.PLAYER_NAMES]: '[gameplay]/playernames',

  [SystemResource.MAP_MANAGER]: '[managers]/mapmanager',
  [SystemResource.SPAWN_MANAGER]: '[managers]/spawnmanager',

  [SystemResource.WEBPACK]: '[system]/[builders]/webpack',
  [SystemResource.YARN]: '[system]/[builders]/yarn',

  [SystemResource.BASE_EVENTS]: '[system]/baseevents',
  [SystemResource.HARD_CAP]: '[system]/hardcap',
  [SystemResource.RCON_LOG]: '[system]/rconlog',
  [SystemResource.RUNCODE]: '[system]/runcode',
  [SystemResource.SESSION_MANAGER]: '[system]/sessionmanager',
});

export const SYSTEM_RESOURCES_DEPENDENCIES: Partial<Record<SystemResource, SystemResource[]>> = Object.freeze({
  [SystemResource.CHAT]: [SystemResource.YARN, SystemResource.WEBPACK],
  [SystemResource.CHAT_THEME_GTAO]: [SystemResource.CHAT],
  [SystemResource.MONEY_FOUNTAIN]: [SystemResource.MAP_MANAGER, SystemResource.MONEY],
  [SystemResource.MONEY_FOUNTAIN_EXAMPLE_MAP]: [SystemResource.MONEY_FOUNTAIN],
});

export const SYSTEM_RESOURCES_NAMES = Object.freeze({
  [SystemResource.MAP_HIPSTER]: 'Map hipster',
  [SystemResource.MAP_SKATER]: 'Map skater',

  [SystemResource.BASIC_GAMEMODE]: 'Basic gamemode',

  [SystemResource.CHAT]: 'Chat',
  [SystemResource.CHAT_THEME_GTAO]: 'GTA:O chat theme',

  [SystemResource.PLAYER_DATA]: 'Player data',
  [SystemResource.PLAYER_NAMES]: 'Player names',

  [SystemResource.MAP_MANAGER]: 'Map manager',
  [SystemResource.SPAWN_MANAGER]: 'Spawn manager',

  [SystemResource.WEBPACK]: 'Webpack builder',
  [SystemResource.YARN]: 'Yarn builder',

  [SystemResource.BASE_EVENTS]: 'Base events',
  [SystemResource.HARD_CAP]: 'Hard cap',
  [SystemResource.RCON_LOG]: 'RCON log',
  [SystemResource.RUNCODE]: 'Runcode',
  [SystemResource.SESSION_MANAGER]: 'Session manager',
});
