import { useServiceOptional } from "cfx/base/servicesContainer";
import { IServersBoostService } from "cfx/common/services/servers/serversBoost.service";
import { IServerView } from "cfx/common/services/servers/types";
import { Button } from "cfx/ui/Button/Button";
import { Title } from "cfx/ui/Title/Title";
import { stopPropagation } from "cfx/utils/domEvents";
import { noop } from "cfx/utils/functional";
import { observer } from "mobx-react-lite";

export interface ServerBoostButtonProps {
  server: IServerView,
  className?: string,
}

export const ServerBoostButton = observer(function ServerBoostButton(props: ServerBoostButtonProps) {
  const {
    server,
    className,
  } = props;

  const ServersBoostService = useServiceOptional(IServersBoostService);
  if (!ServersBoostService) {
    return null;
  }

  const isBoostedByUser = ServersBoostService.currentBoost?.address === server.id;

  const title = isBoostedByUser
    ? `You're BOOSTING™ this server`
    : "Give server a BOOST™!";

  const theme = isBoostedByUser
    ? 'primary'
    : 'default';

  const text = isBoostedByUser
    ? 'BOOSTING™!'
    : 'BOOST™!';

  const handleClick = isBoostedByUser
    ? noop
    : () => ServersBoostService.boostServer(server.id);

  return (
    <Title fixedOn="bottom" title={title}>
      <Button
        size="small"
        theme={theme}
        text={text}
        onClick={stopPropagation(handleClick)}
        disabled={isBoostedByUser}
        className={className}
      />
    </Title>
  );
});
