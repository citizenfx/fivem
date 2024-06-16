/**
 * Core domain-bound logic for different servers-related tasks
 */

import emojiRegex from 'emoji-regex';

import { IServerListConfig, ServersListType } from 'cfx/common/services/servers/lists/types';
import { IAutocompleteIndex } from 'cfx/common/services/servers/source/types';
import { IPinnedServersConfig, IServerView } from 'cfx/common/services/servers/types';

import { isAddressSearchTerm } from './searchTermsParser';

export const EOL_LINK = 'aka.cfx.re/eol';
export const EOS_LINK = 'aka.cfx.re/eos';

export const DEFAULT_SERVER_PORT_INT = 30120;
export const DEFAULT_SERVER_PORT = DEFAULT_SERVER_PORT_INT.toString(10);

export const DEFAULT_SERVER_LOCALE = 'root-AQ';
export const DEFAULT_SERVER_LOCALE_COUNTRY = 'AQ';

const ere = `(?:${emojiRegex().source})`;
const emojiPreRe = new RegExp(`^${ere}`, '');

// 'kush' is a quick hack to prevent non-sentence descriptions
const SPLIT_RE = new RegExp(
  // eslint-disable-next-line @stylistic/max-len
  `((?<!\\.(?:[a-zA-Z]{2,6}))\\s?\\/+\\s?|\\||\\s[-~:x×☆ᆞ]+\\s|\\s[Il]\\s|(?:[\\s⠀ㅤ¦[]|${ere})+(?![#0-9])\\p{Emoji}|(?<=(?!^)(?![#0-9])\\p{Emoji}).+|[・·•│]|(?<=(?:\\]|\\}))[-\\s]|ㅤ|kush|(?<=[】⏌」』]).)`,
  'u',
);
const COMMA_SPLIT_RE = /(?:(?<!(?:\d+|Q))\+|,\s*|\.\s+)/u;

function filterSplit(a: string) {
  const bits = a
    .split(SPLIT_RE)
    .map((b) => b.trim())
    .filter((b) => b !== '');

  return bits.length > 0
    ? bits[0]
    : '';
}

function filterCommas(a: string) {
  const bits = a
    .split(COMMA_SPLIT_RE)
    .map((b) => b.trim())
    .filter((b) => b !== '');

  return bits.slice(0, 3).join(', ');
}

function equalReplace(aRaw: string, ...res: [any, any][]) {
  let lastA: string;
  let a = aRaw;

  do {
    lastA = a;

    for (const re of res) {
      a = a.replace(re[0], re[1]);
    }
  } while (a !== lastA);

  return a;
}

const COUNTRY_PREFIX_RE = /^[[{(][a-zA-Z]{2,}(?:\/...?)*(?:\s.+?)?[\]})]/;

const projectNameReplaces: [RegExp, string | Function][] = [
  [/^[\sㅤ]+/, ''],
  [/(?<=(?!(\d|#))\p{Emoji})(?!(\d|#))\p{Emoji}/u, ''],
  [/^\p{So}/u, ''],
  [/(\s|\u2800)+/gu, ' '],
  [/(?:[0-9]+\+|\+[0-9]+)\s*FPS/g, '+'], // FPS in name
  [/\^[0-9]/, ''], // any non-prefixed color codes
  [/[\])]\s*[[(].*$/, ']'], // suffixes after a tag
  [/,.*$/, ''], // a name usually doesn't contain a comma
  [COUNTRY_PREFIX_RE, ''], // country prefixes
  [emojiPreRe, ''], // emoji prefixes
];
const projectNamesReplacesExtra: [RegExp, string | Function][] = [
  [/[\p{Pe}】]/gu, ''],
  [/(?<!\d)[\p{Ps}【]/gu, ''],
];

/**
 * Returns normalized server name, typically from `sv_projectName` var
 */
export function filterServerProjectName(nameRaw: string | undefined | null): string {
  let name = nameRaw;

  if (!name) {
    return '';
  }

  if (name.length >= 50) {
    name = name.substring(0, 50);
  }

  let colorPrefix = '';

  const filteredName = filterSplit(
    equalReplace(
      equalReplace(
        name,
        [
          /^\^[0-9]/,
          (regs) => {
            colorPrefix = regs;

            return '';
          },
        ],
        ...projectNameReplaces,
      ),
      ...projectNamesReplacesExtra,
    ),
  );

  return colorPrefix + filteredName.normalize('NFKD');
}

/**
 * Returns normalized server description, typically from `sv_projectDesc` var
 */
export function filterServerProjectDesc(aRaw: string | undefined | null): string {
  let a = aRaw;

  if (!a) {
    return '';
  }

  if (a.length >= 125) {
    a = a.substring(0, 125);
  }

  return filterCommas(
    filterSplit(
      equalReplace(
        a,
        [/\^[0-9]/g, ''],
        [/^[\sㅤ]+/, ''],
        [COUNTRY_PREFIX_RE, ''],
        [emojiPreRe, ''], // emoji prefixes
      ),
    ),
  )
    .replace(/(\s|\u2800)+/gu, ' ')
    .normalize('NFKD');
}

export function normalizeSearchString(input: string): string {
  return input.normalize('NFD').replace(/[\u0300-\u036f]/g, '');
}

export function filterServerTag(tag: string) {
  if (!tag) {
    return false;
  }

  switch (tag) {
    case 'default':
      return false;

    default:
      return true;
  }
}

/**
 * Whether or not should gived servers list config prioritize pinned servers when sorting
 */
export function shouldPrioritizePinnedServers(config: IServerListConfig): boolean {
  if (config.prioritizePinned) {
    return true;
  }

  if (config.type === ServersListType.All) {
    if (isAddressSearchTerm(config.searchTextParsed)) {
      return false;
    }

    return Boolean(config.searchText);
  }

  return config.type === ServersListType.Supporters;
}

/**
 * Returns server tags list to render in servers list
 */
export function getListServerTags(server: IServerView, serversIndex: IAutocompleteIndex | null): string[] {
  if (!server.tags) {
    return [];
  }

  if (!serversIndex) {
    return [];
  }

  const tags = serversIndex.tag.items;

  const refinedServerTags: string[] = [];

  for (const serverTag of server.tags) {
    const indexedTag = tags[serverTag];

    if (!indexedTag) {
      continue;
    }

    if (indexedTag.count < 8) {
      continue;
    }

    refinedServerTags.push(serverTag);
  }

  return refinedServerTags.sort((a, b) => tags[b].count - tags[a].count).slice(0, 4);
}

/**
 * Returns pinned server ids list
 */
export function getPinnedServersList(
  pinnedServersConfig: IPinnedServersConfig | null,
  getServer: (id: string) => IServerView | undefined,
): string[] {
  if (!pinnedServersConfig) {
    return [];
  }

  return pinnedServersConfig.pinnedServers
    .filter((address) => getServer(address))
    .sort((a, b) => (getServer(b)?.playersCurrent || 0) - (getServer(a)?.playersCurrent || 0));
}

export function isServerEOL(server: IServerView): boolean {
  // Tue Jun 01 2021 00:00:00 GMT+0200
  // Servers can't be EOL until this date.
  if (new Date().getTime() < 1622498400000) {
    return false;
  }

  if (server.supportStatus === 'unknown') {
    return true;
  }

  if (server.supportStatus === 'end_of_life') {
    return true;
  }

  return false;
}

export function isServerEOS(server: IServerView): boolean {
  if (server.supportStatus === 'end_of_support') {
    return true;
  }

  if (isServerEOL(server)) {
    return false;
  }

  return server.supportStatus === 'end_of_life';
}

const NON_DISPLAY_SERVER_RESOURCE_NAMES = new Set(['_cfx_internal', 'hardcap', 'sessionmanager']);
export function shouldDisplayServerResource(resourceName: string): boolean {
  return !NON_DISPLAY_SERVER_RESOURCE_NAMES.has(resourceName);
}

export const SERVER_PRIVATE_CONNECT_ENDPOINT = 'https://private-placeholder.cfx.re/';

export function hasPrivateConnectEndpoint(endpoints?: string[] | null): boolean {
  if (!endpoints) {
    return false;
  }

  return !notPrivateConnectEndpoint(endpoints[0]);
}

export function notPrivateConnectEndpoint(endpoit: string): boolean {
  return endpoit !== SERVER_PRIVATE_CONNECT_ENDPOINT;
}

export interface IServerConnectEndpoints {
  manual?: string;
  provided?: string[];
}

export function getConnectEndpoits(server: IServerView): IServerConnectEndpoints {
  const eps: IServerConnectEndpoints = {};

  if (server.historicalAddress) {
    eps.manual = server.historicalAddress;
  }

  if (server.connectEndPoints) {
    const provided = server.connectEndPoints.filter(notPrivateConnectEndpoint);

    if (provided.length) {
      eps.provided = provided;
    }
  }

  return eps;
}

export function hasConnectEndpoints(server: IServerView): boolean {
  const endpoints = getConnectEndpoits(server);

  return Boolean(endpoints.manual || endpoints.provided);
}
