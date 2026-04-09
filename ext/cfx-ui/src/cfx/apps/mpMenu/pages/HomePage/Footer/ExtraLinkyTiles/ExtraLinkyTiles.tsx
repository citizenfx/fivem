import { Flex, Icon, Icons, Symbols, Text } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { GameName } from 'cfx/base/game';
import { CurrentGameBrand, currentGameNameIs } from 'cfx/base/gameRuntime';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';

import s from './ExtraLinkyTiles.module.scss';

const analyticsCTAText = `Create a server Find out how to setup your own ${CurrentGameBrand} server!`;

export const ExtraLinkyTiles = observer(function ExtraLinkyTiles() {
  const eventHandler = useEventHandler();

  const link = currentGameNameIs(GameName.RedM)
    ? 'https://redm.net/server-hosting'
    : 'https://fivem.net/server-hosting';

  const handleClick = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.HostCTA,
      properties: {
        element_placement: ElementPlacements.Footer,
        text: analyticsCTAText,
        link_url: link,
      },
    });
  }, [eventHandler, link]);

  return (
    <Flex fullWidth fullHeight alignToEnd gap="large">
      <a href={link} className={s.tile} onClick={handleClick}>
        <Flex gap="large">
          <Flex>
            <Flex vertical gap="normal">
              <Text uppercase size="normal" weight="bold" opacity="75">
                Create a server
              </Text>

              <Text typographic opacity="50">
                Find out how to setup your own&nbsp;{CurrentGameBrand}&nbsp;server!
              </Text>
            </Flex>

            <Icon>
              {Icons.externalLink}
            </Icon>
          </Flex>
        </Flex>
      </a>
    </Flex>
  );
});
