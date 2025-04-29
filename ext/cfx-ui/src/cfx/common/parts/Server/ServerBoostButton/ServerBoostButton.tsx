import { useServiceOptional } from "cfx/base/servicesContainer";
import { $L } from "cfx/common/services/intl/l10n";
import { isServerBoostable } from "cfx/common/services/servers/helpers";
import { IServersBoostService } from "cfx/common/services/servers/serversBoost.service";
import { IServerView } from "cfx/common/services/servers/types";
import { Button, ButtonSize, ButtonTheme } from "cfx/ui/Button/Button";
import { Icons } from "cfx/ui/Icons";
import { Title } from "cfx/ui/Title/Title";
import { stopPropagation } from "cfx/utils/domEvents";
import { noop } from "cfx/utils/functional";
import { observer } from "mobx-react-lite";

export interface ServerBoostButtonProps {
  server: IServerView,
  size?: ButtonSize,
  theme?: ButtonTheme,
  className?: string,
}

export const ServerBoostButton = observer(function ServerBoostButton(props: ServerBoostButtonProps) {
  const {
    server,
    size = 'small',
    className,
  } = props;

  const ServersBoostService = useServiceOptional(IServersBoostService);
  if (!ServersBoostService) {
    return null;
  }

  if (!isServerBoostable(server)) {
    return null;
  }

  const isBoostedByUser = ServersBoostService.currentBoost?.address === server.id;

  const title = isBoostedByUser
    ? $L('#Server_Boost_Title_Active')
    : $L('#Server_Boost_Title');

  const theme = isBoostedByUser
    ? 'primary'
    : props.theme || 'default';

  const text = isBoostedByUser
    ? $L('#Server_Boost_Button_Active')
    : $L('#Server_Boost_Button');

  const handleClick = isBoostedByUser
    ? noop
    : () => ServersBoostService.boostServer(server.id);

  return (
    <Title fixedOn="bottom" title={title}>
      <Button
        size={size}
        theme={theme}
        icon={Icons.serverBoost}
        text={text}
        onClick={stopPropagation(handleClick)}
        disabled={isBoostedByUser}
        className={className}
      />
    </Title>
  );
});
