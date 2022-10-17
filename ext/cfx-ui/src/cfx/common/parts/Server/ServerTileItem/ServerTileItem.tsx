import React from "react";
import { IServerView, ServerViewDetailsLevel } from "cfx/common/services/servers/types";
import { ui } from "cfx/ui/ui";
import { clsx } from "cfx/utils/clsx";
import { observer } from "mobx-react-lite";
import { ServerTitle } from "../ServerTitle/ServerTitle";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { ServerPlayersCount } from "../ServerPlayersCount/ServerPlayersCount";
import { isServerLiveLoading, showServerCountryFlag, showServerPowers } from "cfx/common/services/servers/helpers";
import { CountryFlag } from "cfx/ui/CountryFlag/CountryFlag";
import { ServerIcon } from "../ServerIcon/ServerIcon";
import { Box } from "cfx/ui/Layout/Box/Box";
import { useNavigate } from "react-router-dom";
import { Icons } from "cfx/ui/Icons";
import { Title } from "cfx/ui/Title/Title";
import { ServerPower } from "../ServerPower/ServerPower";
import { ServerBoostButton } from "../ServerBoostButton/ServerBoostButton";
import { ControlBox } from "cfx/ui/ControlBox/ControlBox";
import s from './ServerTileItem.module.scss';
import { ServerConnectButton } from "../ServerConnectButton/ServerConnectButton";
import { ServerFavoriteButton } from "../ServerFavoriteButton/ServerFavoriteButton";
import { preventDefault, stopPropagation } from "cfx/utils/domEvents";
import { Loaf } from "cfx/ui/Loaf/Loaf";
import { Indicator } from "cfx/ui/Indicator/Indicator";

export interface ServerTileItemProps {
  server: IServerView,

  label?: React.ReactNode,
  hideBanner?: boolean,

  growHeight?: boolean,
}

export const ServerTileItem = observer(function ServerTileItem(props: ServerTileItemProps) {
  const {
    server,
    label,
    hideBanner = false,
    growHeight = false,
  } = props;

  const navigate = useNavigate();
  const handleClick = () => navigate(`/servers/detail/${server.id}`);

  const showBanner = !hideBanner && !!server.bannerDetail;

  const showCountryFlag = showServerCountryFlag(server.localeCountry);
  const showPowers = showServerPowers(server);
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
        OFFLINE
      </Loaf>
    );
  } else {
    connectButtonNode = (
      <ServerConnectButton size="normal" theme="transparent" server={server} />
    );
  }

  const rootClassName = clsx(s.root, {
    [s.withBanner]: showBanner,
    [s.withLabel]: !!label,
    [s.growHeight]: growHeight,
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
            <Box height={10}>
              <ServerIcon
                glow
                type="list"
                server={server}
                loading={isLoading}
              />
            </Box>

            <FlexRestricter>
              <Flex vertical fullHeight fullWidth centered="cross-axis" gap="small">
                <ServerTitle
                  truncated
                  size="xlarge"
                  title={server.projectName || server.hostname}
                />

                <Title delay={500} fixedOn="bottom-left" title={server.projectDescription}>
                  <Text truncated opacity="50">
                    {server.projectDescription}
                  </Text>
                </Title>
              </Flex>
            </FlexRestricter>

            {!growHeight && (
              <Flex vertical alignToEndAxis>
                {(showPowers || showCountryFlag) && (
                  <Flex centered>
                    <ServerPower server={server} />

                    {showCountryFlag && (
                      <CountryFlag
                        locale={server.locale}
                        country={server.localeCountry}
                      />
                    )}
                  </Flex>
                )}

                <ControlBox size="small" className={clsx({ [s.hideOnHover]: hidePlayersCountOnHover })}>
                  <Flex centered fullHeight fullWidth>
                    <Text opacity="75">
                      {Icons.playersCount}
                    </Text>
                    <Text opacity="75">
                      <ServerPlayersCount server={server} />
                    </Text>
                  </Flex>
                </ControlBox>

                <ControlBox size="small" className={s.showOnHover}>
                  <Flex>
                    <ServerBoostButton server={server} />

                    <ServerConnectButton size="small" server={server} />

                    <ServerFavoriteButton size="small" server={server} />
                  </Flex>
                </ControlBox>
              </Flex>
            )}
          </Flex>

          {growHeight && (
            <Flex repell centered="axis">
              <ControlBox size="small">
                <Flex centered fullHeight fullWidth>
                  <Text opacity="75">
                    {Icons.playersCount}
                  </Text>
                  <Text opacity="75">
                    <ServerPlayersCount server={server} />
                  </Text>
                </Flex>
              </ControlBox>

              <Flex>
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
