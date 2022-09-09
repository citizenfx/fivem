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
import s from './ServerTileItem.module.scss';
import { $L } from "cfx/common/services/intl/l10n";

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
              <ServerIcon type="list" server={server} />
            </Box>

            <FlexRestricter>
              <Flex vertical fullHeight fullWidth centered="cross-axis" gap="normal">
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

            <div style={{ alignSelf: 'flex-end' }}>
              <Flex vertical alignToEndAxis>
                <Flex centered>
                  {Boolean(server.upvotePower) && (
                    <Title fixedOn="bottom" title={$L('#Server_BoostPower_Title')}>
                      <Flex gap="thin">
                        {Icons.serverBoost}
                        <span>
                          {server.upvotePower}
                        </span>
                      </Flex>
                    </Title>
                  )}

                  {Boolean(server.burstPower) && (
                    <Title fixedOn="bottom" title={$L('#Server_BurstPower_Title')}>
                      <Flex gap="thin">
                        {Icons.serverBurst}
                        <span>
                          {server.burstPower}
                        </span>
                      </Flex>
                    </Title>
                  )}

                  {showCountryFlag && (
                    <CountryFlag
                      locale={server.locale}
                      country={server.localeCountry}
                    />
                  )}
                </Flex>

                <Flex gap="small">
                  {Icons.playersCount}
                  <ServerPlayersCount server={server} />
                </Flex>
              </Flex>
            </div>
          </Flex>
        </Pad>

      </div>
    </div>
  );
});
