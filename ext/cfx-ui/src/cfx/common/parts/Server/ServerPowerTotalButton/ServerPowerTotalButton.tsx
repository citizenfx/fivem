import {
  Button,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { useServiceOptional } from 'cfx/base/servicesContainer';
import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { isServerBoostable } from 'cfx/common/services/servers/helpers';
import { IServersBoostService } from 'cfx/common/services/servers/serversBoost.service';
import { IServerView } from 'cfx/common/services/servers/types';

export interface ServerPowerTotalButtonProps {
  server: IServerView;
  size?: React.ComponentProps<typeof Button>['size'];
  theme?: React.ComponentProps<typeof Button>['theme'];
  className?: string;
  elementPlacement?: ElementPlacements;
}

export const ServerPowerTotalButton = observer(function ServerPowerTotalButton(props: ServerPowerTotalButtonProps) {
  const {
    server,
    size = 'small',
    className,
  } = props;

  const ServersBoostService = useServiceOptional(IServersBoostService);

  if (!ServersBoostService) {
    return null;
  }

  const userBurstPower = ServersBoostService.currentBoost?.burst || 0;
  const userBoostPower = (ServersBoostService.currentBoost?.power || 0) - userBurstPower;

  const isBoostedByUser = ServersBoostService.currentBoost?.address === server.id;

  let title = '';

  if (userBoostPower > 0 && userBurstPower > 0) {
    title = `You are upvoting this server by ${userBoostPower} recurring and ${userBurstPower} burst upvotes.`;
  } else if (userBoostPower > 0 && userBurstPower === 0) {
    title = `You are upvoting this server by ${userBoostPower} recurring upvotes.`;
  } else if (userBoostPower === 0 && userBurstPower > 0) {
    title = `You are upvoting this server by ${userBurstPower} burst upvotes.`;
  }

  const theme = isBoostedByUser
    ? 'primary'
    : props.theme || 'default';

  const totalPower = userBoostPower + userBurstPower;

  if (!isServerBoostable(server)) {
    return null;
  }

  return (
    <Title fixedOn="bottom" title={title}>
      <Button
        size={size}
        theme={theme}
        text={totalPower}
        className={className}
        disabled
      />
    </Title>
  );
});
