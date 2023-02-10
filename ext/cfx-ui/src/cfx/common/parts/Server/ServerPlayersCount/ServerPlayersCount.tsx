import React from "react";
import { IServerView } from "cfx/common/services/servers/types";
import { Symbols } from "cfx/ui/Symbols";

export interface ServerPlayersCountProps {
  server: IServerView,
  serverOtherInstances?: IServerView[] | null,
}

export const ServerPlayersCount = React.forwardRef((props: ServerPlayersCountProps, ref: React.Ref<HTMLSpanElement>) => {
  const {
    server,
    serverOtherInstances
  } = props;

  let playersCurrent = server.playersCurrent;
  let playersMax = server.playersMax;

  if (serverOtherInstances && serverOtherInstances.length > 0) {
    playersCurrent = (playersCurrent || 0) + serverOtherInstances.reduce((total, x) => total + (x.playersCurrent || 0), 0);
    playersMax = (playersMax || 0) + serverOtherInstances.reduce((total, x) => total + (x.playersMax || 0), 0);
  }

  return (
    <span ref={ref}>
      {dashOrNumber(playersCurrent)} / {dashOrNumber(playersMax)}
    </span>
  );
});


function dashOrNumber(num: number | undefined): string | number {
  if (typeof num === 'undefined') {
    return Symbols.longDash;
  }

  return num;
}
