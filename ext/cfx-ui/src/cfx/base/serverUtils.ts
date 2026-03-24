/**
 * Core domain-bound logic for different servers-related tasks
 */

import { splitByIndices } from '@cfx-dev/ui-components';
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

const MAX_LENGTH_PROJECT_NAME = 40;
const MAX_LENGTH_HOSTNAME = 120;
const MAX_LENGTH_PROJECT_DESC = 250;

const ere = `(?:${emojiRegex().source})`;

// 'kush' is a quick hack to prevent non-sentence descriptions
const SPLIT_RE = new RegExp(
  // eslint-disable-next-line @stylistic/max-len
  `((?<!\\.(?:[a-zA-Z]{2,6}))\\s?\\/+\\s?|\\||\\s[-~:x×☆ᆞ]+\\s|\\s[Il]\\s|(?:[\\s⠀ㅤ¦[]|${ere})+(?![#0-9])\\p{Emoji}|(?<=(?!^)(?![#0-9])\\p{Emoji}).+|[・·•│]|(?<=(?:\\]|\\}))[-\\s]|ㅤ|kush|(?<=[】⏌」』]).)`,
  'u',
);
const COMMA_SPLIT_RE = /(?:(?<!(?:\d+|Q))\+|,\s*|\.\s+)/u;

const EMOJI_RE = emojiRegex();

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

type Replacer = [RegExp, string];

// Removes chars from aRaw based on regexps in res repeatedly until no more replacements can be made
function equalReplace(aRaw: string, regexps: Replacer[]) {
  let lastA: string;
  let a = aRaw;

  do {
    lastA = a;

    for (const re of regexps) {
      a = a.replace(re[0], re[1]);
    }
  } while (a !== lastA);

  return a;
}

const COUNTRY_PREFIX_RE = /^[[{(][a-zA-Z]{2,}(?:\/...?)*(?:\s.+?)?[\]})]/;

const projectNameReplaces: Replacer[] = [
  [EMOJI_RE, ''],
  [/^[\sㅤ]+/, ''],
  [/(?<=(?!(\d|#))\p{Emoji})(?!(\d|#))\p{Emoji}/u, ''],
  [/^\p{So}/u, ''],
  [/(\s|\u2800)+/gu, ' '],
  [/(?:[0-9]+\+|\+[0-9]+)\s*FPS/g, '+'], // FPS in name
  [/[\])]\s*[[(].*$/, ']'], // suffixes after a tag
  [/,.*$/, ''], // a name usually doesn't contain a comma
  [COUNTRY_PREFIX_RE, ''], // country prefixes
  [/[\p{Pe}】]/gu, ''],
  [/(?<!\d)[\p{Ps}【]/gu, ''],
];

const projectDescriptionReplaces: Replacer[] = [
  [EMOJI_RE, ''],
  [/^[\sㅤ]+/, ''],
  [COUNTRY_PREFIX_RE, ''],
];

const COLOR_CODES_RE = /\^[0-9]/gu;

function removeColorCodes(input: string): string {
  return input.replace(COLOR_CODES_RE, '');
}

function unicodeLimitLength(input: string, length: number): string {
  return splitByIndices(input, [length], true).get(0) || '';
}

/**
 * Returns normalized server name, typically from `sv_projectName` var
 */
export function filterServerProjectName(nameRaw: string | undefined | null): string {
  if (!nameRaw) {
    return '';
  }

  let name = nameRaw;

  name = removeColorCodes(name);
  name = unicodeLimitLength(name, MAX_LENGTH_PROJECT_NAME);
  name = equalReplace(name, projectNameReplaces);
  name = filterSplit(name);

  return name;
}

export function filterServerHostname(hostnameRaw: string | undefined | null): string {
  if (!hostnameRaw) {
    return '';
  }

  let hostname = hostnameRaw;

  hostname = removeColorCodes(hostname);
  hostname = unicodeLimitLength(hostname, MAX_LENGTH_HOSTNAME);

  return hostname;
}

/**
 * Returns normalized server description, typically from `sv_projectDesc` var
 */
export function filterServerProjectDesc(descriptionRaw: string | undefined | null): string {
  if (!descriptionRaw) {
    return '';
  }

  let description = descriptionRaw;

  description = removeColorCodes(description);
  description = unicodeLimitLength(description, MAX_LENGTH_PROJECT_DESC);
  description = equalReplace(description, projectDescriptionReplaces);

  return description;
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
 * Whether or not should given servers list config prioritize pinned servers when sorting
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

  return false;
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
