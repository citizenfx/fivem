import 'reflect-metadata';
// eslint-disable-next-line import/no-extraneous-dependencies
import { describe, expect, test } from '@jest/globals';

import { getGTMEvent } from './gtm';
import { EventActionNames } from '../types';

describe('trackEvent', () => {
  test('All event names', () => {
    const event = getGTMEvent({
      action: EventActionNames.PageViews,
      properties: { position: 1 },
    });

    expect(event).toStrictEqual({
      event: EventActionNames.PageViews,
      position: 1,
    });
  });
});
