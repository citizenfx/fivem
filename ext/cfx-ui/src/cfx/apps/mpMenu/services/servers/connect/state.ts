import { AdaptiveCard } from 'adaptivecards';

import { MpMenuEvents } from 'cfx/apps/mpMenu/mpMenu.events';
import { IFormattedMessage, maybeParseFormattedMessage } from 'cfx/apps/mpMenu/utils/messageFormatting';
import { RichEvent } from 'cfx/utils/types';

export namespace ConnectState {
  export type DataFor<T> = Omit<T, 'type'>;

  // eslint-disable-next-line no-inner-declarations
  function makeCtor<T>(type: string): (data: DataFor<T>) => T {
    return (data) => ({
      ...data,
      type,
    }) as any as T;
  }

  export type Any = Connecting | Status | Failed | Card | BuildSwitchRequest | BuildSwitchInfo;

  export interface Connecting {
    type: 'connecting';
    generated: boolean;
  }
  export interface Status {
    type: 'status';

    message: string;
    count: number;
    total: number;
    cancelable: boolean;
  }

  export interface Card {
    type: 'card';

    card: string | AdaptiveCard;
  }

  export interface WithFormattableMessage extends IFormattedMessage {
    messageFormatted: boolean;
  }

  export interface Failed extends WithFormattableMessage {
    type: 'failed';

    extra?: RichEvent.Payload<typeof MpMenuEvents.connectFailed>['extra'];
  }

  export interface BuildSwitchRequest {
    type: 'buildSwitchRequest';

    build: number;
    pureLevel: number;
    poolSizesIncrease: string;
    currentBuild: number;
    currentPureLevel: number;
    currentPoolSizesIncrease: string;
  }
  export interface BuildSwitchInfo {
    type: 'buildSwitchInfo';

    title: string;
    content: string;
  }

  // CTORS
  export function connecting(): Connecting {
    return {
      type: 'connecting',
      generated: true,
    };
  }

  export const status = makeCtor<Status>('status');

  export function failed(event: RichEvent.Payload<typeof MpMenuEvents.connectFailed>): Failed {
    const {
      message,
      extra,
    } = event;

    return {
      type: 'failed',
      extra,

      ...maybeParseFormattedMessage(message),
    };
  }

  export const card = makeCtor<Card>('card');
  export const buildSwitchRequest = makeCtor<BuildSwitchRequest>('buildSwitchRequest');
  export const buildSwitchInfo = makeCtor<BuildSwitchInfo>('buildSwitchInfo');
}
