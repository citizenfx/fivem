export enum GameName {
  FiveM = 'gta5',
  RedM = 'rdr3',
  LibertyM = 'ny',

  Launcher = 'launcher',
}

export function getGameBuildDLCName(gameBuild: string): string {
  switch (gameBuild) {
    case '2060':
      return 'Los Santos Summer Special';
    case '2189':
    case '2215':
    case '2245':
      return 'Cayo Perico Heist';
    case '2372':
      return 'Los Santos Tuners';
    case '2545':
    case '2612':
      return 'The Contract';
    case '2699':
      return 'The Criminal Enterprises';
    case '2802':
      return 'Los Santos Drug Wars';
    case '2944':
      return 'San Andreas Mercenaries';
    case '3095':
      return 'The Chop Shop';
    case '3258':
    case '3323':
      return 'Bottom Dollar Bounties';
    case '3407':
      return 'Agents of Sabotage';
  }

  return '';
}

export enum GameUpdateChannel {
  Production = 'production',
  Beta = 'beta',
  Canary = 'canary',
}
