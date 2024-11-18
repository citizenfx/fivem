import { DEFAULT_SERVER_PORT, DEFAULT_SERVER_PORT_INT } from 'cfx/base/serverUtils';

export interface JoinServerAddress {
  type: 'join';
  address: string;
  canonical: string;
}
export function isJoinServerAddress(addr: IParsedServerAddress): addr is JoinServerAddress {
  return addr.type === 'join';
}

export interface JoinOrHostServerAddress {
  type: 'joinOrHost';
  address: string;
  canonical: string;
  addressCandidates: string[];
}
export function isJoinOrHostServerAddress(addr: IParsedServerAddress): addr is JoinOrHostServerAddress {
  return addr.type === 'joinOrHost';
}

export interface IpServerAddress {
  type: 'ip';
  ip: string;
  port: number;
  address: string;
}
export function isIpServerAddress(addr: IParsedServerAddress): addr is IpServerAddress {
  return addr.type === 'ip';
}

export interface HostServerAddress {
  type: 'host';
  address: string;
  addressCandidates?: string[];
}
export function isHostServerAddress(addr: IParsedServerAddress): addr is HostServerAddress {
  return addr.type === 'host';
}

export type IParsedServerAddress = JoinServerAddress | JoinOrHostServerAddress | IpServerAddress | HostServerAddress;

export function parseServerAddress(arbitraryAddressRaw: string): IParsedServerAddress | null {
  const arbitraryAddress = arbitraryAddressRaw.trim();

  if (!arbitraryAddress) {
    return null;
  }

  const arbitraryAddressLowerCase = arbitraryAddress.toLowerCase();

  // Join link first
  const joinLinkDiscriminatorIndex = arbitraryAddressLowerCase.indexOf(JOIN_LINK_DISCRIMINATOR);

  if (joinLinkDiscriminatorIndex > -1) {
    let address = arbitraryAddressLowerCase
      .substring(joinLinkDiscriminatorIndex + JOIN_LINK_DISCRIMINATOR.length)
      .trim();

    // Nothing left - not a valid join link
    if (!address) {
      return null;
    }

    // Could be that there's some junk still
    const indexOfJunk = indexOfFirstNotAlphaNumericChar(address);

    if (indexOfJunk > -1) {
      address = address.substring(0, indexOfJunk);
    }

    return {
      type: 'join',
      address,
      canonical: `https://${JOIN_LINK_DISCRIMINATOR}${address}`,
    };
  }

  // IP address
  const ipParts = tryParseIp(arbitraryAddress);

  if (ipParts) {
    const {
      ip,
      port,
    } = ipParts;

    if (port > 65536) {
      return null;
    }

    return {
      type: 'ip',
      ip,
      port,
      address: ip.includes(':') // if IPv6
        ? `[${ip}]:${port}`
        : `${ip}:${port}`,
    };
  }

  // If only alpha-numeric characters left - assume joinId
  if (indexOfFirstNotAlphaNumericChar(arbitraryAddress) === -1) {
    return {
      type: 'joinOrHost',
      address: arbitraryAddress,
      canonical: `https://${JOIN_LINK_DISCRIMINATOR}${arbitraryAddress}`,
      addressCandidates: [
        `https://${arbitraryAddress}/`,
        `https://${arbitraryAddress}:${DEFAULT_SERVER_PORT}/`,
        `http://${arbitraryAddress}:${DEFAULT_SERVER_PORT}/`,
      ],
    };
  }

  // Inferring address as a host of any form
  try {
    const {
      isBareHost,
      urlNormalized,
    } = normalizeUrl(arbitraryAddress);

    if (!urlNormalized) {
      return null;
    }

    // Parse as URL - it will also perform port range check for us
    const url = new URL(urlNormalized);

    let address = url.href;

    const addressCandidates = [address];

    if (isBareHost) {
      // If no port was specified make address contain it
      // So both `cfx.re` and `cfx.re:30120` would both have main address of `https://cfx.re:30120/`
      if (!url.port) {
        url.port = DEFAULT_SERVER_PORT;
        address = url.href;
        addressCandidates.push(address);
      }

      // Add HTTP variant too
      url.protocol = 'http:';
      addressCandidates.push(url.href);
    }

    if (addressCandidates.length > 1) {
      return {
        type: 'host',
        address,
        addressCandidates,
      };
    }

    return {
      type: 'host',
      address,
    };
  } catch (e) {
    // noop
  }

  return null;
}

/**
 * - replace all \ with /
 * - prepend with https:// if no http(s) protocol provided
 * - remove ?search part
 * - remove #hash part
 * - add a mandatory trailing slash
 * - determine if url is bare host - only domain(:port) is present
 */
function normalizeUrl(str: string) {
  let url = str.replaceAll('\\', '/');

  const hasSlash = url.includes('/');

  const startsWithHTTPProtocol = urlStartsWithHTTPProtocol(url);

  if (!startsWithHTTPProtocol) {
    url = `https://${url}`;
  }

  const indexOfSearch = url.indexOf('?');
  const hasSearch = indexOfSearch > -1;

  if (hasSearch) {
    url = url.substring(0, indexOfSearch);
  }

  const indexOfHash = url.indexOf('#');
  const hasHash = indexOfHash > -1;

  if (hasHash) {
    url = url.substring(0, indexOfHash);
  }

  if (!url.endsWith('/')) {
    url += '/';
  }

  return {
    isBareHost: !hasSlash && !hasSearch && !hasHash,
    urlNormalized: url,
  };
}

function urlStartsWithHTTPProtocol(str: string): boolean {
  const lcstr = str.toLowerCase();

  return lcstr.startsWith('http://') || lcstr.startsWith('https://');
}

function indexOfFirstNotAlphaNumericChar(str: string): number {
  let ptr = -1;

  while (++ptr < str.length) {
    if (!isAlphaNumeric(str.charCodeAt(ptr))) {
      return ptr;
    }
  }

  return -1;
}

function isAlphaNumeric(code: number): boolean {
  const _0 = 48;
  const _9 = 57;
  const _A = 65;
  const _Z = 90;
  const _a = 97;
  const _z = 122;

  if (code > _z) {
    return false;
  }

  if (code >= _a) {
    return true;
  }

  if (code > _Z) {
    return false;
  }

  if (code >= _A) {
    return true;
  }

  if (code > _9) {
    return false;
  }

  if (code >= _0) {
    return true;
  }

  return false;
}

function tryParseIp(str: string): { ip: string; port: number } | null {
  if (isIP(str)) {
    return {
      ip: str,
      port: DEFAULT_SERVER_PORT_INT,
    };
  }

  let ip = '';
  let portString = '';

  // IPv6 with port
  if (str.includes(']:')) {
    [ip, portString] = str.split(']:');

    ip = ip.replace('[', '');
  // IPv4 with port
  } else if (str.includes(':')) {
    [ip, portString] = str.split(':');
  }

  if (!ip || !portString) {
    return null;
  }

  if (isIP(ip)) {
    const port = parseInt(portString, 10) || DEFAULT_SERVER_PORT_INT;

    return {
      ip,
      port,
    };
  }

  return null;
}

const JOIN_LINK_DISCRIMINATOR = 'cfx.re/join/';

// courtesy Node.js source code
// IPv4 Segment
const v4Seg = '(?:[0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])';
const v4Str = `(${v4Seg}[.]){3}${v4Seg}`;
const IPv4Reg = new RegExp(`^${v4Str}$`);

// IPv6 Segment
const v6Seg = '(?:[0-9a-fA-F]{1,4})';
const IPv6Reg = new RegExp(
  '^('
  + `(?:${v6Seg}:){7}(?:${v6Seg}|:)|`
  + `(?:${v6Seg}:){6}(?:${v4Str}|:${v6Seg}|:)|`
  + `(?:${v6Seg}:){5}(?::${v4Str}|(:${v6Seg}){1,2}|:)|`
  + `(?:${v6Seg}:){4}(?:(:${v6Seg}){0,1}:${v4Str}|(:${v6Seg}){1,3}|:)|`
  + `(?:${v6Seg}:){3}(?:(:${v6Seg}){0,2}:${v4Str}|(:${v6Seg}){1,4}|:)|`
  + `(?:${v6Seg}:){2}(?:(:${v6Seg}){0,3}:${v4Str}|(:${v6Seg}){1,5}|:)|`
  + `(?:${v6Seg}:){1}(?:(:${v6Seg}){0,4}:${v4Str}|(:${v6Seg}){1,6}|:)|`
  + `(?::((?::${v6Seg}){0,5}:${v4Str}|(?::${v6Seg}){1,7}|:))`
  + ')(%[0-9a-zA-Z-.:]{1,})?$',
);

function isIPv4(s: string) {
  return IPv4Reg.test(s);
}

function isIPv6(s: string) {
  return IPv6Reg.test(s);
}

function isIP(s: string): 0 | 4 | 6 {
  if (isIPv4(s)) {
    return 4;
  }

  if (isIPv6(s)) {
    return 6;
  }

  return 0;
}

try {
  (window as any).__isIP = isIP;
  (window as any).__tryParseIp = tryParseIp;
  (window as any).__parseServerAddress = parseServerAddress;
} catch (e) {
  // Do nothing
}
