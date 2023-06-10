import { $L } from "cfx/common/services/intl/l10n";
import { IServerView } from "cfx/common/services/servers/types";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Title } from "cfx/ui/Title/Title";
import { observer } from "mobx-react-lite";

export interface ServerPowerProps {
  server: IServerView,

  className?: string,
}

export const ServerPower = observer(function ServerPower(props: ServerPowerProps) {
  const {
    server,
    className,
  } = props;

  const hasUpvote = !!server.upvotePower;
  const hasBurst = !!server.burstPower;

  if (!hasUpvote && !hasBurst) {
    return null;
  }

  return (
    <Flex centered="axis" className={className}>
      {hasUpvote && (
        <Title fixedOn="bottom" title={$L('#Server_BoostPower_Title')}>
          <Flex gap="thin">
            {Icons.serverBoost}
            <span>
              {server.upvotePower}
            </span>
          </Flex>
        </Title>
      )}

      {hasBurst && (
        <Title fixedOn="bottom" title={$L('#Server_BurstPower_Title')}>
          <Flex gap="thin">
            {Icons.serverBurst}
            <span>
              {server.burstPower}
            </span>
          </Flex>
        </Title>
      )}
    </Flex>
  );
});
