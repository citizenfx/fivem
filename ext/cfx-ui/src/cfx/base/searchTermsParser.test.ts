// eslint-disable-next-line import/no-extraneous-dependencies
import { describe, expect, test } from '@jest/globals';

import { searchTermToString } from './searchTermsParser';

describe('searchTermToString', () => {
  test('simple name search term', () => {
    const str = searchTermToString({
      type: 'name',
      source: 'test',
      value: 'test',
      invert: false,
      offset: 0,
      regexp: false,
    });

    expect(str).toEqual('test');
  });

  test('name search term with a quote', () => {
    const str = searchTermToString({
      type: 'name',
      source: 'test',
      value: '"test',
      invert: false,
      offset: 0,
      regexp: false,
    });

    expect(str).toEqual('\\"test');
  });

  test('name search term two quotes', () => {
    const str = searchTermToString({
      type: 'name',
      source: 'test',
      value: '"test"',
      invert: false,
      offset: 0,
      regexp: false,
    });

    expect(str).toEqual('\\"test\\"');
  });

  test('name search term one tilde', () => {
    const str = searchTermToString({
      type: 'name',
      source: 'test',
      value: '~arena',
      invert: false,
      offset: 0,
      regexp: false,
    });

    expect(str).toEqual('\\~arena');
  });

  test('name search term two tilde', () => {
    const str = searchTermToString({
      type: 'name',
      source: 'test',
      value: '~arena~',
      invert: false,
      offset: 0,
      regexp: false,
    });

    expect(str).toEqual('\\~arena\\~');
  });

  test('name search term one backtick', () => {
    const str = searchTermToString({
      type: 'name',
      source: 'test',
      value: '`paradise',
      invert: false,
      offset: 0,
      regexp: false,
    });

    expect(str).toEqual('\\`paradise');
  });

  test('name search term two backtick', () => {
    const str = searchTermToString({
      type: 'name',
      source: 'test',
      value: '`paradise`',
      invert: false,
      offset: 0,
      regexp: false,
    });

    expect(str).toEqual('\\`paradise\\`');
  });
});
