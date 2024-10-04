import {
  CountryFlag,
  Icon,
  Icons,
  InfoPanel,
  Island,
  Box,
  Flex,
  Pad,
  Page,
  Scrollable,
  PremiumBadge,
  Separator,
  Text,
  Linkify,
  clsx,
  identity,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsExclamationTriangleFill, BsLayersFill, BsLockFill, BsTagsFill } from 'react-icons/bs';

import { ALPHANUMERIC_COLLATOR } from 'cfx/base/collators';
import { EOL_LINK, EOS_LINK, isServerEOL, isServerEOS, shouldDisplayServerResource } from 'cfx/base/serverUtils';
import { useService } from 'cfx/base/servicesContainer';
import { ServerFavoriteButton } from 'cfx/common/parts/Server/ServerFavoriteButton/ServerFavoriteButton';
import { ServerIcon } from 'cfx/common/parts/Server/ServerIcon/ServerIcon';
import { ServerPlayersCount } from 'cfx/common/parts/Server/ServerPlayersCount/ServerPlayersCount';
import { ServerPower } from 'cfx/common/parts/Server/ServerPower/ServerPower';
import { useAccountService } from 'cfx/common/services/account/account.service';
import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { useIntlService } from 'cfx/common/services/intl/intl.service';
import { $L, useL10n } from 'cfx/common/services/intl/l10n';
import { getServerLegalRatingImageURL, isServerOffline } from 'cfx/common/services/servers/helpers';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { IServerView, IServerViewPlayer, ServerViewDetailsLevel } from 'cfx/common/services/servers/types';
import { useServerCountryTitle, useTimeoutFlag } from 'cfx/utils/hooks';

import { LongListSideSection } from './LongListSideSection/LongListSideSection';
import { ServerActivityFeed } from '../../parts/Server/ServerActivityFeed/ServerActivityFeed';
import { ServerConnectButton } from '../../parts/Server/ServerConnectButton/ServerConnectButton';
import { ServerExtraDetails } from '../../parts/Server/ServerExtraDetails/ServerExtraDetails';
import { ServerReviews } from '../../parts/Server/ServerReviews/ServerReviews';
import { ServerTitle } from '../../parts/Server/ServerTitle/ServerTitle';

import s from './ServerDetailsPage.module.scss';

const LAYOUT_SPLITS = {
  LEFT: '75%',
  RIGHT: '25%',
};

interface ServerDetailsPageProps {
  server: IServerView;

  forceReviewsAvailable?: boolean;

  onScroll?(event: WheelEvent): void;
  scrollTrackingRef?: React.Ref<HTMLElement>;
}
export const ServerDetailsPage = observer(function Details(props: ServerDetailsPageProps) {
  const {
    server,
    onScroll,
    scrollTrackingRef,
    forceReviewsAvailable = false,
  } = props;

  useEnsureCompleteServerLoaded(server);

  const ServersService = useService(IServersService);

  const shouldShowCompleteServerLoading = useTimeoutFlag(100);
  const isCompleteServerLoading = shouldShowCompleteServerLoading
    && ServersService.isServerLoading(server.id, ServerViewDetailsLevel.MasterListFull);

  const hasBanner = Boolean(server.bannerDetail);

  const displayResources = React.useMemo(() => {
    if (!server.resources) {
      return undefined;
    }

    return server.resources.filter(shouldDisplayServerResource).sort(ALPHANUMERIC_COLLATOR.compare);
  }, [server.resources]);

  const rootClassName = clsx(s.root, 'selectable');
  const bannerStyle: any = hasBanner
    ? { '--banner': `url(${server.bannerDetail})` }
    : {};

  const countryTitle = useServerCountryTitle(server.locale, server.localeCountry);

  const ratingImageURL = getServerLegalRatingImageURL(server);

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
                <Pad size="xlarge">
                  <Flex gap="xlarge">
                    <Flex vertical>
                      <ServerIcon glow type="details" server={server} />

                      <Flex centered className={s.decorator}>
                        {!!server.premium && (
                          <PremiumBadge level={server.premium} />
                        )}

                        {!!server.localeCountry && (
                          <CountryFlag country={server.localeCountry} title={countryTitle} />
                        )}
                      </Flex>

                      <Flex centered>
                        <ServerPower server={server} />
                      </Flex>

                      <Flex centered>
                        <Text opacity="75">{Icons.playersCount}</Text>
                        <Text opacity="75">
                          <ServerPlayersCount server={server} />
                        </Text>
                      </Flex>
                    </Flex>

                    <Box grow>
                      <Flex vertical fullWidth gap="large">
                        <Flex vertical gap="small">
                          <div className={s.title}>
                            <ServerTitle size="xxxlarge" title={server.hostname} />
                          </div>

                          {!!server.projectDescription && (
                            <Text opacity="50">{server.projectDescription}</Text>
                          )}
                        </Flex>

                        <Flex gap="xlarge">
                          <Flex>
                            <div ref={scrollTrackingRef as any}>
                              <ServerConnectButton server={server} elementPlacement={ElementPlacements.ServerPage} />
                            </div>

                            <ServerFavoriteButton size="large" server={server} />
                          </Flex>
                        </Flex>
                      </Flex>
                    </Box>
                  </Flex>
                </Pad>

                <Pad size="xlarge">
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
              <Box style={{
                position: 'sticky',
                top: 0,
              }}
              >
                <Pad top right bottom size="xlarge">
                  <Flex vertical gap="large">
                    {ratingImageURL && (
                      <img src={ratingImageURL} alt="Server Legal Rating" />
                    )}

                    <Warning server={server} />

                    <ServerExtraDetails server={server} />

                    {!!server.players && (
                      <LongListSideSection
                        icon={Icons.playersCount}
                        title={$L('#ServerDetail_Players')}
                        subtitle={`${server.playersCurrent} / ${server.playersMax}`}
                        items={server.players}
                        seeAllTitle={$L('#ServerDetail_Players_ShowAll')}
                        renderPreviewItem={playerRenderer}
                        itemMatchesFilter={(filter, item) => item.name.toLowerCase().includes(filter.toLowerCase())}
                      />
                    )}

                    {!!displayResources && (
                      <LongListSideSection
                        icon={<BsLayersFill />}
                        title={$L('#ServerDetail_Resources')}
                        items={displayResources}
                        seeAllTitle={$L('#ServerDetail_Resources_ShowAll')}
                        renderPreviewItem={identity}
                        itemMatchesFilter={(filter, item) => item.toLowerCase().includes(filter.toLowerCase())}
                      />
                    )}

                    {!!server.tags && (
                      <LongListSideSection
                        icon={<BsTagsFill />}
                        title={$L('#ServerDetail_Tags')}
                        items={server.tags}
                        renderPreviewItem={identity}
                        seeAllTitle={$L('#ServerDetail_Tags_ShowAll')}
                        itemMatchesFilter={(filter, item) => item.toLowerCase().includes(filter.toLowerCase())}
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

  const {
    joinId,
    detailsLevel,
  } = server;

  React.useEffect(() => {
    if (!joinId) {
      return;
    }

    if (server.detailsLevel !== ServerViewDetailsLevel.MasterList) {
      return;
    }

    const to = setTimeout(() => {
      ServersService.loadServerDetailedData(joinId);
    }, 16);

    return () => clearTimeout(to);
  }, [detailsLevel, joinId]);
}

const Warning = observer(function Warning({
  server,
}: {
  server: IServerView;
}) {
  let message: string | null = null;

  const IntlService = useIntlService();
  const AccountService = useAccountService();

  const currentUserIsOwner = server.ownerID === AccountService.account?.id;

  const EOLLinkText = useL10n('#ServerDetail_EOLWarning2_Link', { link: EOL_LINK });
  const EOSLinkText = useL10n('#ServerDetail_SupportWarning2_Link', { link: EOS_LINK });

  if (isServerEOL(server)) {
    const descriptionNode = currentUserIsOwner
      ? (
        <Text typographic size="large">
          {$L('#ServerDetail_EOLWarning2_ForOwner_1')}
          <br />
          <br />
          <Text weight="bold" size="large">
            {$L('#ServerDetail_EOLWarning2_ForOwner_2')}
          </Text>
        </Text>
        )
      : (
        <Text typographic size="large">
          {$L('#ServerDetail_EOLWarning2_ForPlayer_1')}
          <br />
          <br />
          {$L('#ServerDetail_EOLWarning2_ForPlayer_2')}
        </Text>
        );

    return (
      <Flex vertical>
        <InfoPanel size="large" type="error">
          <Pad>
            <Flex vertical gap="large">
              <Icon size="xxlarge" opacity="50">
                <BsExclamationTriangleFill />
              </Icon>

              <Text centered typographic size="xlarge" weight="bold">
                {$L('#ServerDetail_EOLWarning2_Message')}
              </Text>

              <Separator thin />

              {descriptionNode}

              <Separator thin />

              <Text typographic>
                <Linkify text={EOLLinkText} />
              </Text>
            </Flex>
          </Pad>
        </InfoPanel>
      </Flex>
    );
  }

  if (isServerEOS(server)) {
    const descriptionNode = currentUserIsOwner
      ? (
        <Text typographic>
          {$L('#ServerDetail_SupportWarning2_ForOwner_1')}
          <br />
          <br />
          <Text weight="bold">{$L('#ServerDetail_SupportWarning2_ForOwner_2')}</Text>
        </Text>
        )
      : (
        <Text typographic>
          {$L('#ServerDetail_SupportWarning2_ForPlayer_1')}
          <br />
          <br />
          {$L('#ServerDetail_SupportWarning2_ForPlayer_2')}
        </Text>
        );

    const type: React.ComponentProps<typeof InfoPanel>['type'] = currentUserIsOwner
      ? 'warning'
      : 'default';

    return (
      <Flex vertical>
        <InfoPanel type={type}>
          <Pad size="small">
            <Flex vertical gap="normal">
              <Icon size="xlarge" opacity="50">
                <BsExclamationTriangleFill />
              </Icon>

              <Text centered typographic size="large" weight="bold">
                {$L('#ServerDetail_SupportWarning2_Message')}
              </Text>

              <Separator thin />

              {descriptionNode}

              <Separator thin />

              <Text typographic>
                <Linkify text={EOSLinkText} />
              </Text>
            </Flex>
          </Pad>
        </InfoPanel>
      </Flex>
    );
  }

  switch (true) {
    case !!server.private: {
      message = IntlService.translate('#ServerDetail_PrivateWarning');
      break;
    }
    case isServerOffline(server): {
      message = IntlService.translate('#ServerDetail_OfflineWarning');
      break;
    }
  }

  if (!message) {
    return null;
  }

  const icon = server.private
    ? (
      <BsLockFill />
      )
    : (
      <BsExclamationTriangleFill />
      );

  return (
    <InfoPanel size="large" icon={icon}>
      <Linkify text={message} />
    </InfoPanel>
  );
});
