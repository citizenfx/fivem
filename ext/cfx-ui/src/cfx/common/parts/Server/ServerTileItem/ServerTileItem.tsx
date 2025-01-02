import {
  ControlBox,
  CountryFlag,
  Icon,
  Icons,
  Indicator,
  Interactive,
  Box,
  Flex,
  FlexRestricter,
  Loaf,
  Text,
  Title,
  ui,
  clsx,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { useNavigate } from 'react-router-dom';

import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements, isFeaturedElementPlacement } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import {
  getServerDetailsLink,
  isServerLiveLoading,
  showServerCountryFlag,
  showServerPowers,
} from 'cfx/common/services/servers/helpers';
import { IServerView } from 'cfx/common/services/servers/types';
import { preventDefault, stopPropagation } from 'cfx/utils/domEvents';
import { useServerCountryTitle } from 'cfx/utils/hooks';

import { ServerBoostButton } from '../ServerBoostButton/ServerBoostButton';
import { ServerConnectButton } from '../ServerConnectButton/ServerConnectButton';
import { ServerFavoriteButton } from '../ServerFavoriteButton/ServerFavoriteButton';
import { ServerIcon } from '../ServerIcon/ServerIcon';
import { ServerPlayersCount } from '../ServerPlayersCount/ServerPlayersCount';
import { ServerPower } from '../ServerPower/ServerPower';
import { ServerTitle } from '../ServerTitle/ServerTitle';

import s from './ServerTileItem.module.scss';

export interface ServerTileItemProps {
  server: IServerView;

  label?: React.ReactNode;

  hideIcon?: boolean;
  hideBoost?: boolean;
  hideBanner?: boolean;
  hideDescription?: boolean;

  placeControlsBelow?: boolean;
  elementPlacement?: ElementPlacements;
}

export const ServerTileItem = observer(function ServerTileItem(props: ServerTileItemProps) {
  const {
    server,
    label,
    hideIcon = false,
    hideBoost = false,
    hideBanner = false,
    hideDescription = false,
    placeControlsBelow = false,
    elementPlacement = ElementPlacements.Unknown,
  } = props;

  const navigate = useNavigate();
  const eventHandler = useEventHandler();

  const handleClick = React.useCallback(() => {
    const serverLink = getServerDetailsLink(server);

    eventHandler({
      action: EventActionNames.ServerSelect,
      properties: {
        element_placement: elementPlacement,
        server_id: server.id,
        server_name: server.projectName || server.hostname,
        server_type: isFeaturedElementPlacement(elementPlacement)
          ? 'featured'
          : undefined,
        text: 'Server Tile Item',
        link_url: serverLink,
      },
    });
    navigate(serverLink);
  }, [eventHandler, server, elementPlacement]);

  const showBanner = !hideBanner && !!server.bannerDetail;

  const showCountryFlag = showServerCountryFlag(server.localeCountry);
  const showPowers = !hideBoost && showServerPowers(server);
  const hidePlayersCountOnHover = showPowers || showCountryFlag;

  const isLoading = isServerLiveLoading(server);

  let connectButtonNode: React.ReactNode = null;

  if (isLoading) {
    connectButtonNode = (
      <ControlBox>
        <Indicator />
      </ControlBox>
    );
  } else if (server.offline) {
    connectButtonNode = (
      <Loaf color="error">{$L('#Server_Offline')}</Loaf>
    );
  } else {
    connectButtonNode = (
      <ServerConnectButton size="normal" theme="transparent" server={server} elementPlacement={elementPlacement} />
    );
  }

  const rootClassName = clsx(s.root, {
    [s.withBanner]: showBanner,
    [s.withLabel]: !!label,
    [s.placeControlsBelow]: placeControlsBelow,
  });

  const rootStyle: React.CSSProperties = React.useMemo(
    () => ({
      '--banner': ui.url(server.bannerDetail),
    }) as any,
    [server.bannerDetail],
  );

  const countryTitle = useServerCountryTitle(server.locale, server.localeCountry);

  return (
    <Interactive style={rootStyle} className={rootClassName} onClick={preventDefault(stopPropagation(handleClick))}>
      <div className={s.banner} />

      <div className={s.content}>
        <Flex vertical fullHeight>
          {label}

          <Flex fullWidth>
            {!hideIcon && (
              <Box height={10}>
                <ServerIcon type="list" server={server} loading={isLoading} />
              </Box>
            )}

            <FlexRestricter>
              <Flex vertical fullHeight fullWidth centered="cross-axis" gap="small">
                <ServerTitle truncated size="xlarge" title={server.projectName || server.hostname} />

                {!hideDescription && !!server.projectDescription && (
                  <Title delay={500} fixedOn="bottom-left" title={server.projectDescription}>
                    <Text truncated opacity="50">
                      {server.projectDescription}
                    </Text>
                  </Title>
                )}
              </Flex>
            </FlexRestricter>

            {!placeControlsBelow && (
              <Flex vertical alignToEndAxis>
                {(showPowers || showCountryFlag) && (
                  <Flex centered>
                    {showPowers && (
                      <ServerPower server={server} />
                    )}

                    {showCountryFlag && (
                      <CountryFlag title={countryTitle} country={server.localeCountry} />
                    )}
                  </Flex>
                )}

                <ControlBox size="small" className={clsx({ [s.hideOnHover]: hidePlayersCountOnHover })}>
                  <Flex centered fullHeight fullWidth gap="small">
                    <Icon opacity="50">{Icons.playersCount}</Icon>
                    <Text opacity="75">
                      <ServerPlayersCount server={server} />
                    </Text>
                  </Flex>
                </ControlBox>

                <ControlBox size="small" className={s.showOnHover}>
                  <Flex>
                    {!hideBoost && (
                      <ServerBoostButton server={server} elementPlacement={elementPlacement} />
                    )}

                    <ServerFavoriteButton size="small" server={server} />

                    <ServerConnectButton size="small" server={server} elementPlacement={elementPlacement} />
                  </Flex>
                </ControlBox>
              </Flex>
            )}
          </Flex>

          {placeControlsBelow && (
            <Flex repell centered="axis">
              <Flex centered="axis">
                {showCountryFlag && (
                  <CountryFlag title={countryTitle} country={server.localeCountry} />
                )}

                <ControlBox size="small">
                  <Flex centered="axis" fullHeight fullWidth gap="small">
                    <Icon opacity="50">{Icons.playersCount}</Icon>
                    <Text opacity="75">
                      <ServerPlayersCount server={server} />
                    </Text>
                  </Flex>
                </ControlBox>

                {showPowers && (
                  <ServerPower server={server} />
                )}

                {!hideBoost && (
                  <ServerBoostButton
                    size="small"
                    server={server}
                    theme="transparent"
                    className={s.visibleOnHover}
                    elementPlacement={elementPlacement}
                  />
                )}
              </Flex>

              <Flex alignToEndAxis>
                <ServerFavoriteButton size="normal" server={server} />

                {connectButtonNode}
              </Flex>
            </Flex>
          )}
        </Flex>
      </div>
    </Interactive>
  );
});
