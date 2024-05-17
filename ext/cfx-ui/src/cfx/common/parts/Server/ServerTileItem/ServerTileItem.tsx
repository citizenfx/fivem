import React from "react";
import { IServerView } from "cfx/common/services/servers/types";
import { ui } from "cfx/ui/ui";
import { clsx } from "cfx/utils/clsx";
import { observer } from "mobx-react-lite";
import { ServerTitle } from "../ServerTitle/ServerTitle";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { ServerPlayersCount } from "../ServerPlayersCount/ServerPlayersCount";
import { getServerDetailsLink, isServerLiveLoading, showServerCountryFlag, showServerPowers } from "cfx/common/services/servers/helpers";
import { CountryFlag } from "cfx/ui/CountryFlag/CountryFlag";
import { ServerIcon } from "../ServerIcon/ServerIcon";
import { Box } from "cfx/ui/Layout/Box/Box";
import { useNavigate } from "react-router-dom";
import { Icons } from "cfx/ui/Icons";
import { Title } from "cfx/ui/Title/Title";
import { ServerPower } from "../ServerPower/ServerPower";
import { ServerBoostButton } from "../ServerBoostButton/ServerBoostButton";
import { ControlBox } from "cfx/ui/ControlBox/ControlBox";
import { ServerConnectButton } from "../ServerConnectButton/ServerConnectButton";
import { ServerFavoriteButton } from "../ServerFavoriteButton/ServerFavoriteButton";
import { preventDefault, stopPropagation } from "cfx/utils/domEvents";
import { Loaf } from "cfx/ui/Loaf/Loaf";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { $L } from "cfx/common/services/intl/l10n";
import s from './ServerTileItem.module.scss';
import { Icon } from "cfx/ui/Icon/Icon";
import { useEventHandler } from "cfx/common/services/analytics/analytics.service";
import { EventActionNames, ElementPlacements, isFeaturedElementPlacement } from "cfx/common/services/analytics/types";

export interface ServerTileItemProps {
  server: IServerView,

  label?: React.ReactNode,

  hideIcon?: boolean,
  hideBoost?: boolean,
  hideBanner?: boolean,
  hideDescription?: boolean,

  noIconGlow?: boolean,

  placeControlsBelow?: boolean,
  elementPlacement?: ElementPlacements,
}

export const ServerTileItem = observer(function ServerTileItem(props: ServerTileItemProps) {
  const {
    server,
    label,
    hideIcon = false,
    hideBoost = false,
    hideBanner = false,
    hideDescription = false,
    noIconGlow = false,
    placeControlsBelow = false,
    elementPlacement = ElementPlacements.Unknown,
  } = props;

  const navigate = useNavigate();
  const eventHandler = useEventHandler();

  const handleClick = React.useCallback(() => {
    const serverLink = getServerDetailsLink(server);

    eventHandler({ action: EventActionNames.ServerSelect, properties: {
      element_placement: elementPlacement,
      server_id: server.id,
      server_name: server.projectName || server.hostname,
      server_type: isFeaturedElementPlacement(elementPlacement)
        ? 'featured'
        : undefined,
      text: 'Server Tile Item',
      link_url: serverLink,
    }});
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
      <Loaf color="error">
        {$L('#Server_Offline')}
      </Loaf>
    );
  } else {
    connectButtonNode = (
      <ServerConnectButton
        size="normal"
        theme="transparent"
        server={server}
        elementPlacement={elementPlacement}
      />
    );
  }

  const rootClassName = clsx(s.root, {
    [s.withBanner]: showBanner,
    [s.withLabel]: !!label,
    [s.placeControlsBelow]: placeControlsBelow,
  });

  const rootStyle: React.CSSProperties = React.useMemo(() => ({
    '--banner': ui.url(server.bannerDetail),
  } as any), [server.bannerDetail]);

  return (
    <div
      style={rootStyle}
      className={rootClassName}
      onClick={preventDefault(stopPropagation(handleClick))}
    >
      <div className={s.banner} />

      <div className={s.content}>
        <Flex vertical fullHeight>
          {label}

          <Flex fullWidth>
            {!hideIcon && (
              <Box height={10}>
                <ServerIcon
                  glow={!noIconGlow}
                  type="list"
                  server={server}
                  loading={isLoading}
                />
              </Box>
            )}

            <FlexRestricter>
              <Flex vertical fullHeight fullWidth centered="cross-axis" gap="small">
                <ServerTitle
                  truncated
                  size="xlarge"
                  title={server.projectName || server.hostname}
                />

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
                      <CountryFlag
                        locale={server.locale}
                        country={server.localeCountry}
                      />
                    )}
                  </Flex>
                )}

                <ControlBox size="small" className={clsx({ [s.hideOnHover]: hidePlayersCountOnHover })}>
                  <Flex centered fullHeight fullWidth gap="small">
                    <Icon opacity="50">
                      {Icons.playersCount}
                    </Icon>
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
                  <CountryFlag
                    locale={server.locale}
                    country={server.localeCountry}
                  />
                )}

                <ControlBox size="small">
                  <Flex centered="axis" fullHeight fullWidth gap="small">
                    <Icon opacity="50">
                      {Icons.playersCount}
                    </Icon>
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
    </div>
  );
});
