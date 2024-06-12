// eslint-disable-next-line import/no-extraneous-dependencies
import { describe, expect, test } from '@jest/globals';

import { DEFAULT_SERVER_PORT_INT } from 'cfx/base/serverUtils';

import {
  HostServerAddress,
  IParsedServerAddress,
  JoinOrHostServerAddress,
  parseServerAddress,
} from './serverAddressParser';

describe('IP address', () => {
  test('IPv4 address', () => {
    const expected = {
      type: 'ip',
      ip: '127.0.0.1',
      port: DEFAULT_SERVER_PORT_INT,
      address: `127.0.0.1:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parse`127.0.0.1`).toEqual(expected);
    expect(parse`127.0.0.1:30120`).toEqual(expected);
  });

  test('IPv6 address #1', () => {
    const expected = {
      type: 'ip',
      ip: '::1',
      port: DEFAULT_SERVER_PORT_INT,
      address: `[::1]:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parse`::1`).toEqual(expected);
    expect(parse`[::1]:30120`).toEqual(expected);
  });
  test('IPv6 address #2', () => {
    const expected = {
      type: 'ip',
      ip: '2001:db8::123.123.123.123',
      port: DEFAULT_SERVER_PORT_INT,
      address: `[2001:db8::123.123.123.123]:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parse`2001:db8::123.123.123.123`).toEqual(expected);
    expect(parse`[2001:db8::123.123.123.123]:30120`).toEqual(expected);
  });
  test('IPv6 address #3', () => {
    const expected = {
      type: 'ip',
      ip: '::1234:5678:1.2.3.4',
      port: DEFAULT_SERVER_PORT_INT,
      address: `[::1234:5678:1.2.3.4]:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parse`::1234:5678:1.2.3.4`).toEqual(expected);
    expect(parse`[::1234:5678:1.2.3.4]:30120`).toEqual(expected);
  });
  test('IPv6 address #4', () => {
    const expected = {
      type: 'ip',
      ip: '0000:0000:0000:0000:0000:ffff:7f00:0001',
      port: DEFAULT_SERVER_PORT_INT,
      address: `[0000:0000:0000:0000:0000:ffff:7f00:0001]:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parse`0000:0000:0000:0000:0000:ffff:7f00:0001`).toEqual(expected);
    expect(parse`[0000:0000:0000:0000:0000:ffff:7f00:0001]:30120`).toEqual(expected);
  });

  test('out of range IP address port', () => {
    expect(parse`127.0.0.2:100000`).toBe(null);
    expect(parse`[2001:db8::123.123.123.123]:100000`).toBe(null);
  });
});

describe('Join ID or join url', () => {
  test('join link', () => {
    const expected = {
      type: 'join',
      address: 'test',
      canonical: 'https://cfx.re/join/test',
    };

    expect(parse`cfx.re/join/test`).toEqual(expected);
    expect(parse`cfx.re/join/test/stuff`).toEqual(expected);
    expect(parse`cfx.re/join/test/stuff?search`).toEqual(expected);
    expect(parse`https://cfx.re/join/test`).toEqual(expected);
    expect(parse`https://cfx.re/join/test/extra`).toEqual(expected);
    expect(parse`https://cfx.re/join/test/extra?search`).toEqual(expected);
    expect(parse`https://cfx.re/join/test/extra?search#hash`).toEqual(expected);
  });
});

describe('Join ID or host address', () => {
  test('join id or host', () => {
    const expected = {
      type: 'joinOrHost',
      address: 'testie',
      canonical: 'https://cfx.re/join/testie',
      addressCandidates: ['https://testie/', 'https://testie:30120/', 'http://testie:30120/'],
    };

    expect(parse`testie`).toEqual(expected);
  });

  test('candidates equality with bare host', () => {
    const joinIdOrHost = parse`test` as any as JoinOrHostServerAddress;
    const bareHost = parse`test.com` as any as HostServerAddress;

    expect(joinIdOrHost.addressCandidates).toEqual(
      bareHost.addressCandidates?.map((address) => address.replace('.com', '')),
    );
  });
});

describe('Host address', () => {
  test('bare host', () => {
    expect(parse`test.com`).toEqual({
      type: 'host',
      address: 'https://test.com:30120/',
      addressCandidates: ['https://test.com/', 'https://test.com:30120/', 'http://test.com:30120/'],
    });
  });

  test('bare host with port', () => {
    expect(parse`test.com:30120`).toEqual({
      type: 'host',
      address: 'https://test.com:30120/',
      addressCandidates: ['https://test.com:30120/', 'http://test.com:30120/'],
    });
  });

  test('bare host with and without port to resolve to the same address', () => {
    expect(parse`test.com`?.address).toEqual(parse`test.com:30120`?.address);
  });

  test('host with http protocol', () => {
    const expected: IParsedServerAddress = {
      type: 'host',
      address: 'http://test.com/',
    };

    expect(parse`http://test.com`).toEqual(expected);
    expect(parse`http://test.com/`).toEqual(expected);
    expect(parse`http://test.com\\`).toEqual(expected);
    expect(parse`http://test.com?search`).toEqual(expected);
    expect(parse`http://test.com/?search`).toEqual(expected);
    expect(parse`http://test.com#hash`).toEqual(expected);
    expect(parse`http://test.com/#hash`).toEqual(expected);
    expect(parse`http://test.com?search#hash`).toEqual(expected);
    expect(parse`http://test.com/?search#hash`).toEqual(expected);
  });

  test('host with http protocol and port', () => {
    const expected: IParsedServerAddress = {
      type: 'host',
      address: 'http://test.com:30120/',
    };

    expect(parse`http://test.com:30120`).toEqual(expected);
    expect(parse`hTTp://test.com:30120`).toEqual(expected);
    expect(parse`http://test.com:30120\\`).toEqual(expected);
    expect(parse`http://test.com:30120?search`).toEqual(expected);
    expect(parse`http://test.com:30120?search#hash`).toEqual(expected);
    expect(parse`http://test.com:30120#hash`).toEqual(expected);
    expect(parse`http://test.com:30120/`).toEqual(expected);
    expect(parse`http://test.com:30120/?search`).toEqual(expected);
    expect(parse`http://test.com:30120/?search#hash`).toEqual(expected);
    expect(parse`http://test.com:30120/#hash`).toEqual(expected);
  });

  test('host with pathname', () => {
    expect(parse`test.com/`).toEqual({
      type: 'host',
      address: 'https://test.com/',
    });
    expect(parse`test.com:30120/`).toEqual({
      type: 'host',
      address: 'https://test.com:30120/',
    });
    expect(parse`https://test.com/`).toEqual({
      type: 'host',
      address: 'https://test.com/',
    });
    expect(parse`https://test.com:30120/`).toEqual({
      type: 'host',
      address: 'https://test.com:30120/',
    });

    expect(parse`test.com/pathname`).toEqual({
      type: 'host',
      address: 'https://test.com/pathname/',
    });
    expect(parse`test.com:30120/pathname`).toEqual({
      type: 'host',
      address: 'https://test.com:30120/pathname/',
    });
    expect(parse`https://test.com/pathname`).toEqual({
      type: 'host',
      address: 'https://test.com/pathname/',
    });

    expect(parse`test.com/pathname/`).toEqual({
      type: 'host',
      address: 'https://test.com/pathname/',
    });
    expect(parse`https://test.com/pathname/`).toEqual({
      type: 'host',
      address: 'https://test.com/pathname/',
    });
  });

  test('host with search and/or hash', () => {
    const expected: IParsedServerAddress = {
      type: 'host',
      address: 'https://test.com/',
    };

    expect(parse`test.com?`).toEqual(expected);
    expect(parse`test.com?search`).toEqual(expected);
    expect(parse`test.com#`).toEqual(expected);
    expect(parse`test.com#hash`).toEqual(expected);
  });

  test('IDN host', () => {
    expect(parse`ドメイン名例.com`).toEqual({
      type: 'host',
      address: 'https://xn--eckwd4c7cu47r2wf.com:30120/',
      addressCandidates: [
        'https://xn--eckwd4c7cu47r2wf.com/',
        'https://xn--eckwd4c7cu47r2wf.com:30120/',
        'http://xn--eckwd4c7cu47r2wf.com:30120/',
      ],
    });
  });

  test('TLD IDN host', () => {
    expect(parse`ドメイン名例`).toEqual({
      type: 'host',
      address: 'https://xn--eckwd4c7cu47r2wf:30120/',
      addressCandidates: [
        'https://xn--eckwd4c7cu47r2wf/',
        'https://xn--eckwd4c7cu47r2wf:30120/',
        'http://xn--eckwd4c7cu47r2wf:30120/',
      ],
    });
  });
});

describe('Invalid addresses', () => {
  test('invalid address', () => {
    expect(parse`!127.0.0.1`).toBe(null);
    expect(parse`lorem ipsum dolor sit amet`).toBe(null);
    expect(parse`test:com`).toBe(null);
    expect(parse`test.com:65537`).toBe(null);
    expect(parse`#test.com`).toBe(null);
    expect(parse`#test.com#`).toBe(null);
    expect(parse`?test.com`).toBe(null);
    expect(parse`?test.com?`).toBe(null);
  });
});

/**
 * Shorthand for parseServerAddress()
 */
function parse(...[strings, ...substitutes]: Parameters<typeof String.raw>) {
  if (substitutes.length) {
    throw new Error('Use parseServerAddress directly');
  }

  return parseServerAddress(strings[0]);
}
