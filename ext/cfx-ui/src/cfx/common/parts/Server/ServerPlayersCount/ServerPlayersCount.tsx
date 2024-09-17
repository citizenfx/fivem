import { Symbols } from '@cfx-dev/ui-components';
import React from 'react';

import { IServerView } from 'cfx/common/services/servers/types';

export interface ServerPlayersCountProps {
  server: IServerView;
}

export const ServerPlayersCount = React.forwardRef(function ServerPlayersCount(
  props: ServerPlayersCountProps,
  ref: React.Ref<HTMLSpanElement>,
) {
  const {
    server,
  } = props;

  return (
    <span ref={ref}>
      {dashOrNumber(server.playersCurrent)} / {dashOrNumber(server.playersMax)}
    </span>
  );
});

function dashOrNumber(num: number | undefined): string | number {
  if (typeof num === 'undefined') {
    return Symbols.longDash;
  }

  return num;
}
