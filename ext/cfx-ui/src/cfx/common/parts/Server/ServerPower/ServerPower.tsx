import { $L } from "cfx/common/services/intl/l10n";
import { IServerView } from "cfx/common/services/servers/types";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Title } from "cfx/ui/Title/Title";
import { observer } from "mobx-react-lite";

export interface ServerPowerProps {
  server: IServerView,
  serverOtherInstances?: IServerView[] | null,

  className?: string,
}

export const ServerPower = observer(function ServerPower(props: ServerPowerProps) {
  const {
    server,
    serverOtherInstances,
    className,
  } = props;

  let hasUpvote = !!server.upvotePower;
  let hasBurst = !!server.burstPower;

  if (serverOtherInstances && serverOtherInstances.length > 0) {
    hasUpvote = hasUpvote || serverOtherInstances.some(x => !!x.upvotePower);
    hasBurst = hasBurst || serverOtherInstances.some(x => !!x.burstPower);
  }

  if (!hasUpvote && !hasBurst) {
    return null;
  }

  let upvotePower = server.upvotePower;
  let burstPower = server.burstPower;

  if (serverOtherInstances && serverOtherInstances.length > 0) {
    upvotePower = (upvotePower || 0) + serverOtherInstances.reduce((total, x) => total + (x.upvotePower || 0), 0);
    burstPower = (burstPower || 0) + serverOtherInstances.reduce((total, x) => total + (x.burstPower || 0), 0);
  }

  return (
    <Flex centered="axis" className={className}>
      {hasUpvote && (
        <Title fixedOn="bottom" title={$L('#Server_BoostPower_Title')}>
          <Flex gap="thin">
            {Icons.serverBoost}
            <span>
              {upvotePower}
            </span>
          </Flex>
        </Title>
      )}

      {hasBurst && (
        <Title fixedOn="bottom" title={$L('#Server_BurstPower_Title')}>
          <Flex gap="thin">
            {Icons.serverBurst}
            <span>
              {burstPower}
            </span>
          </Flex>
        </Title>
      )}
    </Flex>
  );
});
