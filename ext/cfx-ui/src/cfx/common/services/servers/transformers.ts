import { isFalseString } from '@cfx-dev/ui-components';

import {
  DEFAULT_SERVER_LOCALE,
  DEFAULT_SERVER_LOCALE_COUNTRY,
  filterServerProjectDesc,
  filterServerProjectName,
  filterServerTag,
  hasPrivateConnectEndpoint,
  normalizeSearchString,
} from 'cfx/base/serverUtils';
import { arrayAt } from 'cfx/utils/array';

import { master } from './source/api/master';
import { IArrayCategoryMatcher, IListableServerView, IStringCategoryMatcher } from './source/types';
import {
  IFullServerData,
  IHistoryServer,
  IServer,
  IServerView,
  ServerPureLevel,
  ServerViewDetailsLevel,
} from './types';

// Add new convars to hide here. All sv_* convars filtered out by default.
const convarsToHide = new Set([
  'mapname', 
  'onesync', 
  'gametype', 
]);

export function serverAddress2ServerView(address: string): IServerView {
  const fakeHostname = `⚠️ Server is loading or failed to load (${address}) ⚠️`;

  return {
    id: address,
    detailsLevel: ServerViewDetailsLevel.Address,
    hostname: fakeHostname,
    locale: DEFAULT_SERVER_LOCALE,
    localeCountry: DEFAULT_SERVER_LOCALE_COUNTRY,
    projectName: fakeHostname,
    rawVariables: {},
  };
}

export function masterListServerData2ServerView(joinId: string, data: master.IServerData): IServerView {
  const serverView = Object.assign(
    serverAddress2ServerView(joinId),
    {
      joinId,
      detailsLevel: ServerViewDetailsLevel.MasterList,
      enforceGameBuild: data.vars?.sv_enforceGameBuild,
      gametype: data.gametype,
      mapname: data.mapname,
      server: data.server,
      hostname: data.hostname || '',
      playersMax: data.svMaxclients || 0,
      playersCurrent: data.clients || 0,
      burstPower: data.burstPower || 0,
      upvotePower: data.upvotePower || 0,
      connectEndPoints: data.connectEndPoints,
      private: hasPrivateConnectEndpoint(data.connectEndPoints),
      rawVariables: data.vars || {},
    },
    processServerDataVariables(data.vars),
  );

  if (Object.prototype.hasOwnProperty.call(data, 'iconVersion')) {
    serverView.iconVersion = data.iconVersion;
  }

  if (!serverView.projectName) {
    serverView.upvotePower = 0;
  }

  return serverView;
}

export function masterListFullServerData2ServerView(joinId: string, data: IFullServerData['Data']): IServerView {
  const serverView = Object.assign(
    serverAddress2ServerView(joinId),
    {
      joinId,
      detailsLevel: ServerViewDetailsLevel.MasterListFull,
      enforceGameBuild: data.vars?.sv_enforceGameBuild,
      gametype: data.gametype,
      mapname: data.mapname,
      server: data.server,
      hostname: data.hostname || '',
      playersMax: data.svMaxclients || 0,
      playersCurrent: data.clients || 0,
      burstPower: data.burstPower || 0,
      upvotePower: data.upvotePower || 0,
      connectEndPoints: data.connectEndPoints,

      private: data.private || hasPrivateConnectEndpoint(data.connectEndPoints),

      ownerID: data.ownerID,
      ownerName: data.ownerName,
      ownerAvatar: data.ownerAvatar,
      ownerProfile: data.ownerProfile,

      supportStatus: (data.support_status as any) || 'supported',

      resources: data.resources as any,
      players: data.players as any,

      rawVariables: data.vars || {},
    },
    processServerDataVariables(data.vars),
  );

  if (Object.prototype.hasOwnProperty.call(data, 'iconVersion')) {
    serverView.iconVersion = data.iconVersion;
  }

  if (!serverView.projectName) {
    serverView.upvotePower = 0;
  }

  if (data.fallback) {
    serverView.offline = true;
  }

  return serverView;
}

export function historyServer2ServerView(historyServer: IHistoryServer): IServerView {
  const server: IServerView = {
    id: historyServer.address,
    detailsLevel: ServerViewDetailsLevel.Historical,
    locale: DEFAULT_SERVER_LOCALE,
    localeCountry: DEFAULT_SERVER_LOCALE_COUNTRY,
    hostname: historyServer.hostname,
    projectName: historyServer.hostname,
    rawVariables: historyServer.vars,
    historicalIconURL: historyServer.rawIcon,
  };

  return Object.assign(server, processServerDataVariables(historyServer.vars));
}

export function serverView2ListableServerView(server: IServerView): IListableServerView {
  const playersCurrent = server.playersCurrent || 0;
  const playersMax = server.playersMax || 0;

  const searchableName = getSearchableName(server);
  const sortableName = getSortableName(searchableName);

  return {
    id: server.id,

    ping: 0,

    players: playersCurrent,
    isFull: playersCurrent >= playersMax,
    isEmpty: playersCurrent === 0,

    locale: server.locale,

    tags: server.tags || [],
    tagsMap: server.tags
      ? server.tags.reduce((acc, tag) => {
        acc[tag] = true;

        return acc;
      }, {})
      : {},

    variables: server.variables || {},

    searchableName,
    sortableName,

    premium: server.premium || '',
    upvotePower: server.upvotePower || 0,

    categories: createCategoryMatchers(server),
  };
}

function getSearchableName(server: IServerView): string {
  const name = server.projectDescription
    ? `${server.projectName} ${server.projectDescription}`
    : server.projectName;

  return normalizeSearchString(name.replace(/\^[0-9]/g, ''));
}

function getSortableName(searchableName: string): string {
  return searchableName
    .replace(/[^a-zA-Z0-9]/g, '')
    .replace(/^[0-9]+/g, '')
    .toLowerCase();
}

function shouldVarBeShown(key: string): boolean {
  return !convarsToHide.has(key) && !key.startsWith('sv_');
}

type VarsView = Partial<
  Pick<
    IServerView,
    | 'tags'
    | 'locale'
    | 'premium'
    | 'gamename'
    | 'canReview'
    | 'variables'
    | 'pureLevel'
    | 'projectName'
    | 'bannerDetail'
    | 'rawVariables'
    | 'localeCountry'
    | 'onesyncEnabled'
    | 'activitypubFeed'
    | 'licenseKeyToken'
    | 'bannerConnecting'
    | 'enforceGameBuild'
    | 'scriptHookAllowed'
    | 'projectDescription'
  >
>;

export function processServerDataVariables(vars?: IServer['data']['vars']): VarsView {
  const view: VarsView = {
    projectName: '',
  };

  if (!vars) {
    return view;
  }

  view.variables = {};

  for (const [key, value] of Object.entries(vars)) {
    const lckey = key.toLowerCase();

    switch (true) {
      case key === 'sv_projectName': {
        view.projectName = filterServerProjectName(value);
        continue;
      }
      case key === 'sv_projectDesc': {
        view.projectDescription = filterServerProjectDesc(value);
        continue;
      }
      case key === 'sv_licenseKeyToken': {
        view.licenseKeyToken = value;
        continue;
      }
      case key === 'sv_scriptHookAllowed': {
        view.scriptHookAllowed = value === 'true';
        continue;
      }
      case key === 'gamename': {
        view.gamename = value;
        continue;
      }
      case key === 'activitypubFeed': {
        view.activitypubFeed = value;
        continue;
      }
      case key === 'premium': {
        view.premium = value;
        continue;
      }
      case key === 'locale': {
        view.locale = getCanonicalLocale(value);
        view.localeCountry = arrayAt(view.locale.split('-'), -1) || '??';
        continue;
      }
      case key === 'tags': {
        view.tags = [
          ...new Set(
            value
              .split(',')
              .map((tag) => tag.trim().toLowerCase())
              .filter(filterServerTag),
          ),
        ];
        continue;
      }
      case key === 'banner_connecting': {
        view.bannerConnecting = value;
        continue;
      }
      case key === 'banner_detail': {
        view.bannerDetail = value;
        continue;
      }
      case key === 'can_review': {
        view.canReview = Boolean(value);
        continue;
      }
      case key === 'onesync_enabled': {
        view.onesyncEnabled = value === 'true';
        continue;
      }
      case key === 'sv_enforceGameBuild': {
        if (value) {
          view.enforceGameBuild = value;
        }
        continue;
      }
      case key === 'sv_pureLevel': {
        view.pureLevel = value as ServerPureLevel;
        continue;
      }
      case !shouldVarBeShown(key): {
        continue;
      }
      case lckey.includes('banner_'):
      case lckey.includes('sv_project'):
      case lckey.includes('version'):
      case lckey.includes('uuid'): {
        continue;
      }
    }

    view.variables![key] = value;
  }

  return view;
}

function createCategoryMatchers(server: IServerView) {
  const {
    id,
    tags,
    locale,
    gamename,
    gametype,
    mapname,
    hostname,
    enforceGameBuild,
    pureLevel,
    rawVariables,
  } = server;

  const categories: IListableServerView['categories'] = {
    address: createStringMatcher(id),
  };

  if (locale) {
    categories.locale = createStringMatcher(locale);
  }

  if (hostname) {
    categories.hostname = createStringMatcher(hostname);
  }

  if (gamename) {
    categories.gamename = createStringMatcher(gamename);
  }

  if (gametype) {
    categories.gametype = createStringMatcher(gametype);
  }

  if (mapname) {
    categories.mapname = createStringMatcher(mapname);
  }

  if (enforceGameBuild) {
    categories.gamebuild = createStringMatcher(enforceGameBuild);
  }

  if (pureLevel) {
    categories.purelevel = createStringMatcher(pureLevel);
  }

  if (tags && tags.length) {
    categories.tag = createArrayMatcher(tags);
  }

  // Doesn't really make sense as no custom variables data in the master list, for now
  if (rawVariables) {
    const truthyVars = Object.entries(rawVariables)
      .filter(([, value]) => !isFalseString(value))
      .map(([key]) => key);

    if (truthyVars.length) {
      categories.var = createArrayMatcher(truthyVars);
    }

    for (const [varName, varValue] of Object.entries(rawVariables)) {
      // Don't overwrite existing
      if (categories[varName]) {
        continue;
      }

      categories[varName] = createStringMatcher(varValue);
    }
  }

  return categories;
}

function createStringMatcher(against: string): IStringCategoryMatcher {
  return {
    type: 'string',
    against,
  };
}

function createArrayMatcher(against: string[]): IArrayCategoryMatcher {
  return {
    type: 'array',
    against,
  };
}

function getCanonicalLocale(locale: string): string {
  try {
    return Intl.getCanonicalLocales(locale.replace(/_/g, '-'))[0];
  } catch {
    return DEFAULT_SERVER_LOCALE;
  }
}
