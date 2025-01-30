import { Flex, Text } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { FiServer } from 'react-icons/fi';

import { GameName } from 'cfx/base/game';
import { CurrentGameBrand, currentGameNameIs } from 'cfx/base/gameRuntime';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';

import s from './ExtraLinkyTiles.module.scss';

export const ExtraLinkyTiles = observer(function ExtraLinkyTiles() {
  const eventHandler = useEventHandler();

  const link = currentGameNameIs(GameName.RedM)
    ? 'https://redm.net/server-hosting'
    : 'https://fivem.net/server-hosting';
  const title = 'Create a server';
  const description = `Find out how to setup your own ${CurrentGameBrand} server!`;

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
    <Flex fullWidth alignToEnd gap="large">
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
