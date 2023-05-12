import React from "react";
import { IServerView } from "cfx/common/services/servers/types";
import { Symbols } from "cfx/ui/Symbols";

export interface ServerPlayersCountProps {
  server: IServerView,
}

export const ServerPlayersCount = React.forwardRef((props: ServerPlayersCountProps, ref: React.Ref<HTMLSpanElement>) => {
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
