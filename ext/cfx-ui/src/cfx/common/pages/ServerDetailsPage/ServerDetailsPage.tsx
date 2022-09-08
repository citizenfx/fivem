import React from "react";
import { isServerEOL, isServerEOS, shouldDisplayServerResource } from "cfx/base/serverUtils";
import { useService } from "cfx/base/servicesContainer";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { IServerView, IServerViewPlayer, ServerViewDetailsLevel } from "cfx/common/services/servers/types";
import { Button } from "cfx/ui/Button/Button";
import { CountryFlag } from "cfx/ui/CountryFlag/CountryFlag";
import { Icons } from "cfx/ui/Icons";
import { InfoPanel } from "cfx/ui/InfoPanel/InfoPanel";
import { Island } from "cfx/ui/Island/Island";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Page } from "cfx/ui/Layout/Page/Page";
import { Scrollable } from "cfx/ui/Layout/Scrollable/Scrollable";
import { PremiumBadge } from "cfx/ui/PremiumBadge/PremiumBadge";
import { Text } from "cfx/ui/Text/Text";
import { clsx } from "cfx/utils/clsx";
import { Linkify } from "cfx/utils/links";
import { observer } from "mobx-react-lite";
import { BsExclamationTriangleFill, BsLayersFill, BsLockFill, BsTagsFill } from "react-icons/bs";
import { ServerActivityFeed } from "../../parts/Server/ServerActivityFeed/ServerActivityFeed";
import { ServerConnectButton } from "../../parts/Server/ServerConnectButton/ServerConnectButton";
import { ServerExtraDetails } from "../../parts/Server/ServerExtraDetails/ServerExtraDetails";
import { ServerReviews } from "../../parts/Server/ServerReviews/ServerReviews";
import { ServerTitle } from "../../parts/Server/ServerTitle/ServerTitle";
import { LongListSideSection } from "./LongListSideSection/LongListSideSection";
import { ServerIcon } from "cfx/common/parts/Server/ServerIcon/ServerIcon";
import { playSfx, Sfx } from "cfx/apps/mpMenu/utils/sfx";
import { useIntlService } from "cfx/common/services/intl/intl.service";
import { ALPHANUMERIC_COLLATOR } from "cfx/base/collators";
import s from './ServerDetailsPage.module.scss';
import { useTimeoutFlag } from "cfx/utils/hooks";

const LAYOUT_SPLITS = {
  LEFT: '75%',
  RIGHT: '25%'
};

interface ServerDetailsPageProps {
  server: IServerView,

  forceReviewsAvailable?: boolean,

  onScroll?(event: WheelEvent): void,
  scrollTrackingRef?: React.Ref<HTMLElement>,
}
export const ServerDetailsPage = observer(function Details(props: ServerDetailsPageProps) {
  const {
    server,
    onScroll,
    scrollTrackingRef,
    forceReviewsAvailable = false,
  } = props;

  // if (__CFXUI_USE_SOUNDS__) {
  //   React.useEffect(() => playSfx(Sfx.Woosh3), []);
  // }

  useEnsureCompleteServerLoaded(server);

  const { address } = server;
  const ServersService = useService(IServersService);

  const shouldShowCompleteServerLoading = useTimeoutFlag(300);
  const isCompleteServerLoading = shouldShowCompleteServerLoading && ServersService.isServerLoading(server.address, ServerViewDetailsLevel.Complete);

  const isOffline = Boolean(server.offline);
  const hasBanner = Boolean(server.bannerDetail);

  const displayResources = React.useMemo(() => {
    if (!server.resources) {
      return undefined;
    }

    return server.resources
      .filter(shouldDisplayServerResource)
      .sort(ALPHANUMERIC_COLLATOR.compare);
  }, [server.resources]);

  const rootClassName = clsx(s.root, 'selectable');
  const bannerStyle: any = hasBanner
    ? { '--banner': `url(${server.bannerDetail})` }
    : {};

  return (
    <Page showLoader={isCompleteServerLoading}>
      <Island grow className={rootClassName}>
        <Scrollable onScroll={onScroll}>
          {hasBanner && (
            <div className={s.banner} style={bannerStyle} />
          )}

          <Flex fullWidth gap="none">
            <Box grow>
              <Flex vertical gap="large">
                <Pad size="large">
                  <Flex gap="xlarge">
                    <Flex vertical>
                      <ServerIcon
                        type="details"
                        server={server.address}
                      />

                      <Flex centered className={s.decorator}>
                        {!!server.premium && (
                          <PremiumBadge level={server.premium} />
                        )}

                        {!!server.localeCountry && (
                          <CountryFlag country={server.localeCountry} locale={server.locale} />
                        )}
                      </Flex>

                      {!isOffline && (
                        <Flex centered>
                          <Text opacity="75">
                            {Icons.playersCount}
                          </Text>
                          <Text opacity="75">
                            {server.playersCurrent} / {server.playersMax}
                          </Text>
                        </Flex>
                      )}
                    </Flex>

                    <Box grow>
                      <Flex vertical fullWidth gap="large">
                        <Flex vertical gap="small">
                          <div className={s.title}>
                            <ServerTitle size="xxxlarge" title={server.hostname} />
                          </div>

                          {!!server.projectDescription && (
                            <Text opacity="50">
                              {server.projectDescription}
                            </Text>
                          )}
                        </Flex>

                        <Flex gap="xlarge">
                          <Flex>
                            <div ref={scrollTrackingRef as any}>
                              <ServerConnectButton server={server} />
                            </div>

                            <Favorite address={address} />
                          </Flex>

                          <Warning server={server} />
                        </Flex>
                      </Flex>
                    </Box>
                  </Flex>
                </Pad>

                <Pad size="large">
                  <Flex vertical gap="large">
                    {server.activitypubFeed && (
                      <ServerActivityFeed pub={server.activitypubFeed} />
                    )}

                    {(server.canReview || forceReviewsAvailable) && (
                      <ServerReviews server={server} />
                    )}
                  </Flex>
                </Pad>
              </Flex>
            </Box>

            <Box grow={false} width={LAYOUT_SPLITS.RIGHT}>
              <Box style={{ position: 'sticky', top: 0 }}>
                <Pad size="large">
                  <Flex vertical gap="large">
                    <ServerExtraDetails server={server} />

                    {/* <Separator /> */}

                    {!!server.players && (
                      <LongListSideSection
                        icon={Icons.playersCount}
                        title="Players"
                        subtitle={`${server.playersCurrent} / ${server.playersMax}`}
                        items={server.players}
                        seeAllTitle="Show all players"
                        renderPreviewItem={playerRenderer}
                        itemMatchesFilter={(filter, item) => item.name.toLowerCase().includes(filter)}
                      />
                    )}

                    {!!displayResources && (
                      <LongListSideSection
                        icon={<BsLayersFill />}
                        title="Resources"
                        items={displayResources}
                        seeAllTitle="Show all resources"
                        renderPreviewItem={(item) => item}
                        itemMatchesFilter={(filter, item) => item.toLowerCase().includes(filter)}
                      />
                    )}

                    {!!server.tags && (
                      <LongListSideSection
                        icon={<BsTagsFill />}
                        title="Tags"
                        items={server.tags}
                        renderPreviewItem={(item) => item}
                        seeAllTitle="Show all tags"
                        itemMatchesFilter={(filter, item) => item.toLowerCase().includes(filter)}
                      />
                    )}
                  </Flex>
                </Pad>
              </Box>
            </Box>

          </Flex>
        </Scrollable>
      </Island>
    </Page>
  );
});

function playerRenderer(player: IServerViewPlayer): React.ReactNode {
  return player.name;
}

function useEnsureCompleteServerLoaded(server: IServerView) {
  const ServersService = useService(IServersService);

  React.useEffect(() => {
    let to = setTimeout(() => {
      if (server.detailsLevel < ServerViewDetailsLevel.Complete) {
        ServersService.loadServerDetailedData(server.address);
      }
    }, 16);

    return () => clearTimeout(to);
  }, [server]);
}


const Favorite = observer(function Favorite({ address }: { address: string }) {
  const ServersService = useService(IServersService);
  const favoriteServersList = ServersService.getFavoriteList();

  if (!favoriteServersList) {
    return null;
  }

  const isInFavoriteServersList = favoriteServersList.isIn(address);

  const handleClick = () => {
    if (__CFXUI_USE_SOUNDS__) {
      if (isInFavoriteServersList) {
        playSfx(Sfx.Click4);
      } else {
        playSfx(Sfx.Click2);
      }
    }

    favoriteServersList.toggleIn(address);
  };

  return (
    <Button
      size="large"
      icon={
        isInFavoriteServersList
          ? Icons.favoriteActive
          : Icons.favoriteInactive
      }
      onClick={handleClick}
    />
  );
});

const Warning = observer(function Warning({ server }: { server: IServerView }) {
  let message: string | null = null;

  const IntlService = useIntlService();

  switch (true) {
    case !!server.private: {
      message = IntlService.translate('#ServerDetail_PrivateWarning');
      break;
    }
    case isServerEOL(server): {
      message = IntlService.translate('#ServerDetail_EOLWarning');
      break;
    }
    case isServerEOS(server): {
      message = IntlService.translate('#ServerDetail_SupportWarning');
      break;
    }
    case server.offline:
    case !!server.fallback: {
      message = IntlService.translate('#ServerDetail_OfflineWarning');
      break;
    }
  }

  if (!message) {
    return null;
  }

  const icon = server.private
    ? <BsLockFill />
    : <BsExclamationTriangleFill />;

  return (
    <InfoPanel size="large" icon={icon}>
      <Linkify text={message} />
    </InfoPanel>
  );
});
