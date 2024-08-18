import {
  Button,
  ControlBox,
  Icon,
  Indicator,
  Island,
  Box,
  Flex,
  Page,
  Text,
  Title,
  ui,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsArrowLeft } from 'react-icons/bs';
import { TbMoodCry } from 'react-icons/tb';
import { Navigate, useNavigate, useParams } from 'react-router-dom';

import { useService } from 'cfx/base/servicesContainer';
import { ServerDetailsPage } from 'cfx/common/pages/ServerDetailsPage/ServerDetailsPage';
import { ServerBackdropBanner } from 'cfx/common/parts/Server/ServerBackdropBanner/ServerBackdropBanner';
import { ServerConnectButton } from 'cfx/common/parts/Server/ServerConnectButton/ServerConnectButton';
import { ServerReviewFormContext } from 'cfx/common/parts/Server/ServerReviews/ServerReviewForm/ServerReviewForm';
import { ServerTitle } from 'cfx/common/parts/Server/ServerTitle/ServerTitle';
import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { IServersService, useServersService } from 'cfx/common/services/servers/servers.service';
import { IServerView } from 'cfx/common/services/servers/types';
import { LinkButton } from 'cfx/ui/Button/LinkButton';
import { useTimeoutFlag } from 'cfx/utils/hooks';

import { usePageScrollSync } from './usePageScrollSync';
import { InsideNavBar } from '../../parts/NavBar/InsideNavBar';
import { useForceTransparentNav } from '../../parts/NavBar/NavBarState';
import { useStreamerMode } from '../../services/convars/convars.service';

export const MpMenuServerDetailsPage = observer(function MpMenuServerDetailsPage() {
  const {
    '*': address,
  } = useParams();
  const ServersService = useService(IServersService);

  if (!address) {
    return (
      <Navigate to="/servers" />
    );
  }

  const server = ServersService.getServer(address);

  if (!server) {
    if (ServersService.serversListLoading) {
      return (
        <Loader />
      );
    }

    return (
      <NotFound />
    );
  }

  return (
    <DetailsWrapper server={server!} />
  );
});

function Loader() {
  // If we're in loading state for less than 300ms -
  // showing loader would only be distracting
  const showIndicator = useTimeoutFlag(300);

  return (
    <Page>
      <Island className={ui.cls.fullHeight}>
        <Flex centered className={ui.cls.fullHeight}>
          {showIndicator && (
            <Indicator />
          )}
        </Flex>
      </Island>
    </Page>
  );
}

function NotFound() {
  return (
    <Page>
      <Island className={ui.cls.fullHeight}>
        <Flex centered vertical className={ui.cls.fullHeight}>
          <Icon size="xxlarge" color="error">
            <TbMoodCry style={{
              fontSize: '10em',
              opacity: 0.5,
            }}
            />
          </Icon>

          <Text size="xxlarge">{$L('#ServerDetail_NotFound')}</Text>

          <div />
          <div />
          <div />
          <div />
          <div />

          <LinkButton to="/servers" text={$L('#ServerDetail_NotFound_AllServersLink')} />
        </Flex>
      </Island>
    </Page>
  );
}

const DetailsWrapper = observer(function DetailsWrapper({
  server,
}: { server: IServerView }) {
  const ServersService = useServersService();

  useForceTransparentNav();

  const navigate = useNavigate();

  const connectButtonRef = React.useRef<HTMLButtonElement>(null);
  const {
    handleScroll,
    navBarTranslateY,
  } = usePageScrollSync(connectButtonRef); // backdropTranslateY

  const streamerMode = useStreamerMode();

  return (
    <>
      <InsideNavBar>
        <Box noOverflow>
          <Flex gap="large">
            <Title title={$L('#ServerDetail_Back')}>
              <Button size="large" icon={<BsArrowLeft />} onClick={() => navigate(-1)} />
            </Title>

            <Box grow>
              <ControlBox size="large">
                <ServerTitle title={server.projectName} size="xlarge" />
              </ControlBox>
            </Box>

            <Box style={{ transform: `translateY(${navBarTranslateY})` }}>
              <ServerConnectButton server={server} elementPlacement={ElementPlacements.ServerPage} />
            </Box>
          </Flex>
        </Box>
      </InsideNavBar>

      <ServerBackdropBanner
        animated
        server={server}
        // offsetY={backdropTranslateY}
      />

      <ServerReviewFormContext.Provider value={{ censorUser: streamerMode }}>
        <ServerDetailsPage
          server={server}
          onScroll={handleScroll}
          scrollTrackingRef={connectButtonRef}
          forceReviewsAvailable={ServersService.isServerPinned(server.id)}
        />
      </ServerReviewFormContext.Provider>
    </>
  );
});
