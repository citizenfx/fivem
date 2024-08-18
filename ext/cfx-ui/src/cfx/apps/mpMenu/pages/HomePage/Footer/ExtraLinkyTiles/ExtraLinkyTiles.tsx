import { Flex, Text } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { FiServer } from 'react-icons/fi';

import { GameName } from 'cfx/base/game';
import { currentGameNameIs } from 'cfx/base/gameRuntime';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';

import s from './ExtraLinkyTiles.module.scss';

export const ExtraLinkyTiles = observer(function ExtraLinkyTiles() {
  const eventHandler = useEventHandler();

  const link = 'https://docs.fivem.net/docs/server-manual/setting-up-a-server/';
  const title = 'Host a server';
  const description = 'Find out how to host a server on hardware you control';

  const handleClick = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.HostCTA,
      properties: {
        element_placement: ElementPlacements.Footer,
        text: `${title} ${description}`,
        link_url: link,
      },
    });
  }, [eventHandler, link, title, description]);

  return (
    <Flex fullWidth gap="large">
      <StartYourServerPromo />

      <a href={link} className={s.tile} onClick={handleClick}>
        <Flex gap="large">
          <div className={s.icon}>
            <FiServer />
          </div>

          <Flex vertical gap="small">
            <Text size="xlarge" weight="bold" family="secondary" opacity="75">
              {title}
            </Text>

            <Text opacity="50">{description}</Text>
          </Flex>
        </Flex>
      </a>
    </Flex>
  );
});

const zapLogoImageURL = new URL('assets/images/zap.jpg', import.meta.url).toString();

export const StartYourServerPromo = observer(function StartYourServerPromo() {
  const eventHandler = useEventHandler();
  const isFiveM = currentGameNameIs(GameName.FiveM);
  const isRedM = currentGameNameIs(GameName.RedM);

  if (!isFiveM && !isRedM) {
    return null;
  }

  const link = isFiveM
    ? 'https://zap-hosting.com/fivemsl'
    : 'https://zap-hosting.com/redm2';
  const title = 'Start a server';
  const description = 'Rent a server at ZAP-Hosting';

  // eslint-disable-next-line react-hooks/rules-of-hooks
  const handleClick = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.StartCTA,
      properties: {
        element_placement: ElementPlacements.Footer,
        text: `${title} ${description}`,
        link_url: link,
      },
    });
  }, [eventHandler, link, title, description]);

  return (
    <a href={link} className={s.tile} onClick={handleClick}>
      <Flex gap="large">
        <div className={s.icon}>
          <img src={zapLogoImageURL} alt="" />
        </div>

        <Flex vertical gap="small">
          <Text size="xlarge" weight="bold" family="secondary" opacity="75">
            {title}
          </Text>

          <Text opacity="50">{description}</Text>
        </Flex>
      </Flex>
    </a>
  );
});
