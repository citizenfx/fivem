import {describe, expect, test} from '@jest/globals';
import { DEFAULT_SERVER_PORT_INT } from 'cfx/base/serverUtils';
import { IParsedServerAddress, parseServerAddress } from './serverAddressParser';

describe('serverAddressParser', () => {
  test('parses IPv4 address', () => {
    const expected = {
      type: 'ip',
      ip: '127.0.0.1',
      port: DEFAULT_SERVER_PORT_INT,
      address: `127.0.0.1:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parseServerAddress('127.0.0.1')).toEqual(expected);
    expect(parseServerAddress('127.0.0.1:30120')).toEqual(expected);
  });

  test('parses IPv6 address #1', () => {
    const expected = {
      type: 'ip',
      ip: '::1',
      port: DEFAULT_SERVER_PORT_INT,
      address: `[::1]:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parseServerAddress('::1')).toEqual(expected);
    expect(parseServerAddress('[::1]:30120')).toEqual(expected);
  });
  test('parses IPv6 address #2', () => {
    const expected = {
      type: 'ip',
      ip: '2001:db8::123.123.123.123',
      port: DEFAULT_SERVER_PORT_INT,
      address: `[2001:db8::123.123.123.123]:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parseServerAddress('2001:db8::123.123.123.123')).toEqual(expected);
    expect(parseServerAddress('[2001:db8::123.123.123.123]:30120')).toEqual(expected);
  });
  test('parses IPv6 address #3', () => {
    const expected = {
      type: 'ip',
      ip: '::1234:5678:1.2.3.4',
      port: DEFAULT_SERVER_PORT_INT,
      address: `[::1234:5678:1.2.3.4]:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parseServerAddress('::1234:5678:1.2.3.4')).toEqual(expected);
    expect(parseServerAddress('[::1234:5678:1.2.3.4]:30120')).toEqual(expected);
  });
  test('parses IPv6 address #4', () => {
    const expected = {
      type: 'ip',
      ip: '0000:0000:0000:0000:0000:ffff:7f00:0001',
      port: DEFAULT_SERVER_PORT_INT,
      address: `[0000:0000:0000:0000:0000:ffff:7f00:0001]:${DEFAULT_SERVER_PORT_INT}`,
    };

    expect(parseServerAddress('0000:0000:0000:0000:0000:ffff:7f00:0001')).toEqual(expected);
    expect(parseServerAddress('[0000:0000:0000:0000:0000:ffff:7f00:0001]:30120')).toEqual(expected);
  });

  test('out of range IP address port', () => {
    expect(parseServerAddress('127.0.0.2:100000')).toBe(null);
    expect(parseServerAddress('[2001:db8::123.123.123.123]:100000')).toBe(null);
  });

  test('parses join link', () => {
    const expected = {
      type: 'join',
      address: 'test',
      canonical: 'https://cfx.re/join/test',
    };

    expect(parseServerAddress('cfx.re/join/test')).toEqual(expected);
    expect(parseServerAddress('cfx.re/join/test/stuff')).toEqual(expected);
    expect(parseServerAddress('cfx.re/join/test/stuff?utm=false')).toEqual(expected);
    expect(parseServerAddress('https://cfx.re/join/test')).toEqual(expected);
    expect(parseServerAddress('https://cfx.re/join/test/bullshit')).toEqual(expected);
    expect(parseServerAddress('https://cfx.re/join/test/bullshit?utm=true')).toEqual(expected);
  });

  test('parses join id', () => {
    const expected = {
      type: 'join',
      address: 'testie',
      canonical: 'https://cfx.re/join/testie',
    };

    expect(parseServerAddress('testie')).toEqual(expected);
  });

  test('parses host without port', () => {
    const expected: IParsedServerAddress = {
      type: 'host',
      address: 'test.com',
    };

    expect(parseServerAddress('test.com')).toEqual(expected);
    expect(parseServerAddress('test.com')).toEqual(expected);
    expect(parseServerAddress('test.com/pathname?search=string')).toEqual(expected);
    expect(parseServerAddress('test.com/pathname?search=string#hash')).toEqual(expected);
    expect(parseServerAddress('test.com/pathname#test')).toEqual(expected);
    expect(parseServerAddress('nope://test.com/pathname#test')).toEqual(expected);
    expect(parseServerAddress('https://test.com')).toEqual(expected);
    expect(parseServerAddress('http://test.com')).toEqual(expected);
    expect(parseServerAddress('fivem://test.com')).toEqual(expected);
    expect(parseServerAddress('ftp://test.com')).toEqual(expected);

    // with junk, but still parseable
    expect(parseServerAddress('ftp://test.com\\')).toEqual(expected);
    expect(parseServerAddress('ftp://test.com++')).toEqual(expected);
    expect(parseServerAddress('test test.com')).toEqual(expected);
    expect(parseServerAddress('test test.com ')).toEqual(expected);
    expect(parseServerAddress('test test.com something else')).toEqual(expected);
  });

  test('parses host with port', () => {
    const expected: IParsedServerAddress = {
      type: 'host',
      address: 'test.com:30120',
    };

    expect(parseServerAddress('test.com:30120')).toEqual(expected);
    expect(parseServerAddress('test.com:30120/pathname?search=string')).toEqual(expected);
    expect(parseServerAddress('test.com:30120/pathname?search=string#hash')).toEqual(expected);
    expect(parseServerAddress('test.com:30120/pathname#test')).toEqual(expected);
    expect(parseServerAddress('nope://test.com:30120/pathname#test')).toEqual(expected);
    expect(parseServerAddress('https://test.com:30120')).toEqual(expected);
    expect(parseServerAddress('http://test.com:30120')).toEqual(expected);
    expect(parseServerAddress('fivem://test.com:30120')).toEqual(expected);
    expect(parseServerAddress('ftp://test.com:30120')).toEqual(expected);

    // with junk, but still parseable
    expect(parseServerAddress('ftp://test.com:30120\\')).toEqual(expected);
    expect(parseServerAddress('ftp://test.com:30120++')).toEqual(expected);
    expect(parseServerAddress('test test.com:30120')).toEqual(expected);
    expect(parseServerAddress('test test.com:30120')).toEqual(expected);
    expect(parseServerAddress('test test.com:30120 ')).toEqual(expected);
    expect(parseServerAddress('test test.com:30120 something else')).toEqual(expected);
  });

  test('parses IDN host', () => {
    expect(parseServerAddress('ドメイン名例.com')).toEqual({
      type: 'host',
      address: 'xn--eckwd4c7cu47r2wf.com',
    })
  });

  test('does not parse junk', () => {
    expect(parseServerAddress('ドメイン名例')).toBe(null);
    expect(parseServerAddress('!127.0.0.1')).toBe(null);
    expect(parseServerAddress('test/test')).toBe(null);
    expect(parseServerAddress('lorem ipsum dolor sit amet')).toBe(null);
  });
});
