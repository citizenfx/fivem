import React from "react";
import { IServerView } from "cfx/common/services/servers/types";
import { ui } from "cfx/ui/ui";
import { clsx } from "cfx/utils/clsx";
import { observer } from "mobx-react-lite";
import { ServerTitle } from "../ServerTitle/ServerTitle";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { ServerPlayersCount } from "../ServerPlayersCount/ServerPlayersCount";
import { showServerCountryFlag } from "../ServerListItem/utils";
import { CountryFlag } from "cfx/ui/CountryFlag/CountryFlag";
import { ServerIcon } from "../ServerIcon/ServerIcon";
import { Box } from "cfx/ui/Layout/Box/Box";
import { useNavigate } from "react-router-dom";
import { Icons } from "cfx/ui/Icons";
import { Title } from "cfx/ui/Title/Title";
import { ServerPower } from "../ServerPower/ServerPower";
import s from './ServerTileItem.module.scss';
import { ServerBoostButton } from "../ServerBoostButton/ServerBoostButton";
import { ControlBox } from "cfx/ui/ControlBox/ControlBox";

export interface ServerTileItemProps {
  server: IServerView,
}

export const ServerTileItem = observer(function ServerTileItem(props: ServerTileItemProps) {
  const {
    server,
  } = props;

  const navigate = useNavigate();
  const handleClick = React.useCallback(() => {
    navigate(`/servers/detail/${server.id}`);
  }, [navigate, server]);

  const showCountryFlag = showServerCountryFlag(server.localeCountry);

  const rootClassName = clsx(s.root, {
    [s.withBanner]: server.bannerDetail,
  });

  const rootStyle: React.CSSProperties = React.useMemo(() => ({
    '--banner': ui.url(server.bannerDetail),
  } as any), [server.bannerDetail]);

  return (
    <div
      style={rootStyle}
      className={rootClassName}
      onClick={handleClick}
    >
      <div className={s.banner} />

      <div className={s.content}>
        <Pad left right bottom top={!server.bannerDetail}>
          <Flex fullWidth>
            <Box height={10}>
              <ServerIcon glow type="list" server={server} />
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

            <div style={{ alignSelf: 'center' }}>
              <Flex vertical alignToEndAxis gap="small">
                <Flex centered>
                  <ServerPower server={server} />

                  {showCountryFlag && (
                    <CountryFlag
                      locale={server.locale}
                      country={server.localeCountry}
                    />
                  )}
                </Flex>

                <ControlBox size="small" className={s.showOnHover}>
                  <ServerBoostButton server={server} />
                </ControlBox>

                <ControlBox size="small" className={s.hideOnHover}>
                  <Flex centered fullHeight fullWidth>
                    <Text opacity="75">
                      {Icons.playersCount}
                    </Text>
                    <Text opacity="75">
                      <ServerPlayersCount server={server} />
                    </Text>
                  </Flex>
                </ControlBox>
              </Flex>
            </div>
          </Flex>
        </Pad>

      </div>
    </div>
  );
});
