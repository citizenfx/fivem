import { Flex, Icon, Icons, Text, Title } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { $L } from 'cfx/common/services/intl/l10n';
import { IServerView } from 'cfx/common/services/servers/types';

import s from './ServerPower.module.scss';

export interface ServerPowerProps {
  server: IServerView;

  className?: string;
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
          <Flex centered gap="small">
            <Icon className={s.upvoteIcon}>
              {Icons.upvotesRecurring}
            </Icon>
            <Text size="normal" opacity="75">
              {server.upvotePower}
            </Text>
          </Flex>
        </Title>
      )}

      {hasBurst && (
        <Title fixedOn="bottom" title={$L('#Server_BurstPower_Title')}>
          <Flex centered gap="small">
            <Icon className={s.burstIcon}>
              {Icons.upvotesBurst}
            </Icon>
            <Text size="normal" opacity="75">
              {server.burstPower}
            </Text>
          </Flex>
        </Title>
      )}
    </Flex>
  );
});
