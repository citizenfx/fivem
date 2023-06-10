import React from "react";
import { ServerBackdropBanner } from "cfx/common/parts/Server/ServerBackdropBanner/ServerBackdropBanner";
import { ServerConnectButton } from "cfx/common/parts/Server/ServerConnectButton/ServerConnectButton";
import { ServerDetailsPage } from "cfx/common/pages/ServerDetailsPage/ServerDetailsPage";
import { ServerTitle } from "cfx/common/parts/Server/ServerTitle/ServerTitle";
import { useService } from "cfx/base/servicesContainer";
import { IServersService, useServersService } from "cfx/common/services/servers/servers.service";
import { IServerView } from "cfx/common/services/servers/types";
import { ControlBox } from "cfx/ui/ControlBox/ControlBox";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Page } from "cfx/ui/Layout/Page/Page";
import { observer } from "mobx-react-lite";
import { BsArrowLeft } from "react-icons/bs";
import { Navigate, useNavigate, useParams } from "react-router-dom";
import { InsideNavBar } from "../../parts/NavBar/InsideNavBar";
import { usePageScrollSync } from "./usePageScrollSync";
import { Title } from "cfx/ui/Title/Title";
import { Button } from "cfx/ui/Button/Button";
import { Island } from "cfx/ui/Island/Island";
import { Icon } from "cfx/ui/Icon/Icon";
import { TbMoodCry } from 'react-icons/tb';
import { Text } from "cfx/ui/Text/Text";
import { ui } from "cfx/ui/ui";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { useTimeoutFlag } from "cfx/utils/hooks";
import { useStreamerMode } from "../../services/convars/convars.service";
import { ServerReviewFormContext } from "cfx/common/parts/Server/ServerReviews/ServerReviewForm/ServerReviewForm";
import { useForceTransparentNav } from "../../parts/NavBar/NavBarState";
import { $L } from "cfx/common/services/intl/l10n";

export const MpMenuServerDetailsPage = observer(function MpMenuServerDetailsPage() {
  const { '*': address } = useParams();
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
            <TbMoodCry style={{ fontSize: '10em', opacity: .5 }} />
          </Icon>

          <Text size="xxlarge">
            {$L('#ServerDetail_NotFound')}
          </Text>

          <div />
          <div />
          <div />
          <div />
          <div />

          <LinkButton
            to="/servers"
            text={$L('#ServerDetail_NotFound_AllServersLink')}
          />
        </Flex>
      </Island>
    </Page>
  );
}

const DetailsWrapper = observer(function DetailsWrapper({ server }: { server: IServerView }) {
  const ServersService = useServersService();

  useForceTransparentNav();

  const navigate = useNavigate();

  const connectButtonRef = React.useRef<HTMLButtonElement>(null);
  const { handleScroll, backdropTranslateY, navBarTranslateY } = usePageScrollSync(connectButtonRef);

  const streamerMode = useStreamerMode();

  return (
    <>
      <InsideNavBar>
        <Box noOverflow>
          <Flex gap="large">
            <Title title={$L('#ServerDetail_Back')}>
              <Button
                size="large"
                icon={<BsArrowLeft />}
                onClick={() => navigate(-1)}
              />
            </Title>

            <Box grow>
              <ControlBox size="large">
                <ServerTitle
                  title={server.projectName}
                  size="xlarge"
                />
              </ControlBox>
            </Box>

            <Box style={{ transform: `translateY(${navBarTranslateY})` }}>
              <ServerConnectButton server={server} />
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
