export enum GameMode {
  NORMAL,
  WORLD_EDITOR,
}

export function getGameMode(): GameMode {
  if (GetConvarInt('sdk_worldEditorMode', 0) === 1) {
    return GameMode.WORLD_EDITOR;
  }

  return GameMode.NORMAL;
}

export enum ServerMode {
  NORMAL,
  LEGACY,
}

export function getServerMode(): ServerMode {
  if (GetConvarInt('sv_fxdkServerMode', 0) === 1) {
    return ServerMode.NORMAL;
  }

  return ServerMode.LEGACY;
}

export function joaat(key): string {
  key = key.toLowerCase();

  const hash = new Uint32Array(1);

  for (const i in key) {
    hash[0] += key.charCodeAt(i);
    hash[0] += hash[0] << 10;
    hash[0] ^= hash[0] >>> 6;
  }

  hash[0] += hash[0] << 3;
  hash[0] ^= hash[0] >>> 11;
  hash[0] += hash[0] << 15;

  return '0x' + hash[0].toString(16).toUpperCase();
}
