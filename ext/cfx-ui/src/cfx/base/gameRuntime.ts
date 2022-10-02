import { GameName } from "cfx/base/game";

const gameBrandMap = {
  [GameName.FiveM]: 'FiveM',
  [GameName.RedM]: 'RedM',
  [GameName.LibertyM]: 'LibertyM',
};
function getGameBrand(gameName: GameName): string {
  return gameBrandMap[gameName] || 'CitizenFX';
}

export let CurrentGameName = GameName.FiveM;
export let CurrentGameBrand = getGameBrand(CurrentGameName);
export let CurrentGameBuild = '-1';
export let CurrentGamePureLevel = '-1';

export function currentGameNameIs(gameName: GameName): boolean {
  return CurrentGameName === gameName;
}

export function setCurrentGameName(gameName: GameName) {
  CurrentGameName = gameName;
  CurrentGameBrand = getGameBrand(CurrentGameName);
}

export function setCurrentGameBuild(gameBuild: string) {
  CurrentGameBuild = gameBuild;
}

export function setCurrentGamePureLevel(pureLevel: string) {
  CurrentGamePureLevel = pureLevel;
}
