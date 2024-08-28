import { GameName } from 'cfx/base/game';

const gameBrandMap = {
  [GameName.FiveM]: 'FiveM',
  [GameName.RedM]: 'RedM',
  [GameName.LibertyM]: 'LibertyM',
};
function getGameBrand(gameName: GameName): string {
  return gameBrandMap[gameName] || 'CitizenFX';
}

const gameProtocolMap = {
  [GameName.FiveM]: 'fivem',
  [GameName.RedM]: 'redm',
};
function getGameProtocol(gameName: GameName): string | undefined {
  return gameProtocolMap[gameName];
}

// eslint-disable-next-line import/no-mutable-exports
export let CurrentGameName = GameName.FiveM;
// eslint-disable-next-line import/no-mutable-exports
export let CurrentGameBrand = getGameBrand(CurrentGameName);
// eslint-disable-next-line import/no-mutable-exports
export let CurrentGameProtocol = getGameProtocol(CurrentGameName);
// eslint-disable-next-line import/no-mutable-exports
export let CurrentGameBuild = '-1';
// eslint-disable-next-line import/no-mutable-exports
export let CurrentGamePureLevel = '-1';

export function currentGameNameIs(gameName: GameName): boolean {
  return CurrentGameName === gameName;
}

export function setCurrentGameName(gameName: GameName) {
  CurrentGameName = gameName;
  CurrentGameBrand = getGameBrand(CurrentGameName);
  CurrentGameProtocol = getGameProtocol(CurrentGameName);
}

export function setCurrentGameBuild(gameBuild: string) {
  CurrentGameBuild = gameBuild;
}

export function setCurrentGamePureLevel(pureLevel: string) {
  CurrentGamePureLevel = pureLevel;
}
