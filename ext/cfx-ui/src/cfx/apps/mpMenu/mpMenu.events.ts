import { RichEvent } from 'cfx/utils/types';

export namespace MpMenuEvents {
  export const connectTo = RichEvent.define<{
    hostnameStr: string;
    connectParams: string;
  }>('connectTo');

  export const backfillServerInfo = RichEvent.define<{
    data: {
      nonce: string;
      server: {
        icon: string;
        token: string;
        vars: Record<string, string>;
      };
    };
  }>('backfillServerInfo');

  export const connecting = RichEvent.define('connecting');

  export const connectStatus = RichEvent.define<{
    data: {
      message: string;
      count: number;
      total: number;
      cancelable: boolean;
    };
  }>('connectStatus');

  export const connectFailed = RichEvent.define<{
    message: string;
    extra?:
      | { fault: 'you'; status?: true; action: string }
      | { fault: 'cfx'; status: true; action: string }
      | { fault: 'server'; status?: true; action: string }
      | { fault: 'either'; status: true; action: string };
  }>('connectFailed');
}
