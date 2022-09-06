import { Icons } from "cfx/ui/Icons";
import { Page } from "cfx/ui/Layout/Page/Page";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { InsideNavBar } from "cfx/apps/mpMenu/parts/NavBar/InsideNavBar";
import { observer } from "mobx-react-lite";
import { HomePageBottomLinks, HomePageNavBarLinks } from "./HomePage.links";
import { Island } from "cfx/ui/Island/Island";
import { Text } from "cfx/ui/Text/Text";
import { CurrentGameName, currentGameNameIs } from "cfx/base/gameName";
import { Feed } from "./Feed/Feed";
import { GameName } from "cfx/base/game";
import { useService } from "cfx/base/servicesContainer";
import { ServersListType } from "cfx/common/services/servers/lists/types";
import { VirtualScrollable } from "cfx/ui/Layout/Scrollable/VirtualScrollable";
import { Box } from "cfx/ui/Layout/Box/Box";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { ui } from "cfx/ui/ui";
import { ServerListItem } from "cfx/common/parts/Server/ServerListItem/ServerListItem";
import { PlatformStatus } from "./PlatformStatus/PlatformStatus";
import { MpMenuServersService } from "../../services/servers/servers.mpMenu";
import { useActivityService } from "cfx/common/services/activity/activity.service";
import { useUiService } from "cfx/common/services/ui/ui.service";
import { IndexedServerListItem } from "cfx/common/parts/Server/ServerListItem/IndexedServerListItem";
import { FaMedal, FaRetweet, FaServer } from "react-icons/fa";
import { FiTwitter } from "react-icons/fi";
import { Icon } from "cfx/ui/Icon/Icon";
import { StoryMode } from "./StoryMode/StoryMode";
import { ReplayEditor } from "./ReplayEditor/ReplayEditor";
import { MpMenuLocalhostServerService } from "../../services/servers/localhostServer.mpMenu";
import { IServersConnectService } from "cfx/common/services/servers/serversConnect.service";
import { serversListsEhers } from "cfx/common/pages/ServersPage/ListTypeTabs";
import { Title } from "cfx/ui/Title/Title";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { Button } from "cfx/ui/Button/Button";
import { ImMagicWand } from 'react-icons/im';
import { WiStars } from 'react-icons/wi';
import s from './HomePage.module.scss';
import { useSavedScrollPositionForBackNav } from "cfx/utils/hooks";
import { $L } from "cfx/common/services/intl/l10n";

// #TODOLOC
const SERVERS_LIST_TITLE = (
  <>
    Servers are sorted in this order:
    <br />
    <br />
    {Icons.serversListHistory} Most recent server connected to
    <br />
    {Icons.serversListFavorites} Favorite servers
    <br />
    {Icons.serversListAll} All other servers
  </>
);

export const HomePage = observer(function HomePage() {
  const ActivityService = useActivityService();

  return (
    <Page>
      <InsideNavBar>
        <Flex gap="large">
          <HomePageNavBarLinks />

          <PlatformStatus />
        </Flex>
      </InsideNavBar>

      <Flex fullWidth fullHeight gap="large">
        <Flex fullWidth vertical gap="large">
          <ServersContainer />

          {currentGameNameIs(GameName.FiveM) && (
            <PromotedServer />
          )}

          <Flex repell>
            {(CurrentGameName === GameName.FiveM) && (
              <Flex>
                <StoryMode />
                <ReplayEditor />
              </Flex>
            )}

            <HomePageBottomLinks />
          </Flex>
        </Flex>

        <Flex vertical fullHeight>
          <FlexRestricter vertical>
            <Flex fullHeight gap="thin" className={s.feeds}>
              <Feed
                icon={<FaRetweet />}
                items={ActivityService.communityItems}
                label={$L('#Home_Feed_Community')}
                title={$L('#Home_Feed_Community_Desc')}
              />

              <Feed
                icon={<FiTwitter />}
                items={ActivityService.officialItems}
                label={$L('#Home_Feed_Official')}
                title={$L('#Home_Feed_Official_Desc')}
              />
            </Flex>
          </FlexRestricter>
        </Flex>
      </Flex>
    </Page >
  );
});

const PromotedServerHeaders = {
  ad: {
    label: $L('#Home_PromotedServer_Ad'),
    icon: Icons.serversStaffPickUnstyled,
  },
  locale: {
    label: $L('#Home_PromotedServer_Locale'),
    icon: <FaMedal />,
  },
};
const PromotedServer = observer(function PromotedServer() {
  const ServersService = useService(MpMenuServersService);

  if (!ServersService.promotedServerDescriptor) {
    return null;
  }

  const label = PromotedServerHeaders[ServersService.promotedServerDescriptor.origin];

  return (
    // <Island>
    // <Pad>
    <Flex vertical>
      <Flex centered="axis">
        <Icon size="large" opacity="50">
          {label.icon}
        </Icon>

        <Text weight="bold" size="large" opacity="50">
          {label.label}
        </Text>
      </Flex>

      <Flex fullWidth>
        <FlexRestricter>
          <Box height={12}>
            <ServerListItem
              standalone
              descriptionUnderName
              server={ServersService.getServer(ServersService.pinnedServersConfig?.noAdServerId!)}
            />
          </Box>
        </FlexRestricter>
      </Flex>
    </Flex>
    // </Pad>
    // </Island>
  );
});

const ServersContainer = observer(function ServersContainer() {
  const UiService = useUiService();
  const ServersService = useService(MpMenuServersService);
  const LocalhostServer = useService(MpMenuLocalhostServerService);
  const ServersConnectService = useService(IServersConnectService);

  const serversList = ServersService.getList(ServersListType.MildlyIntelligent)!;

  const [initialScrollOffset, setScrollOffset] = useSavedScrollPositionForBackNav('home-screen');

  const list = serversList.sequence;

  return (
    <FlexRestricter vertical>
      <Flex vertical fullHeight>
        <Flex repell centered="axis">
          <Title title={SERVERS_LIST_TITLE}>
            <Flex centered="axis" gap="normal">
              <Icon size="large" opacity="50">
                <ImMagicWand />
                {/* <WiStars /> */}
              </Icon>

              <Text size="large" weight="bold" opacity="50">
                {$L('#Home_PersonalizedList')}
              </Text>
            </Flex>
          </Title>

          <Flex>
            {!!LocalhostServer.address && (
              <Title title={$L('#Home_LocalServer_Title', { name: LocalhostServer.displayName })}>
                <Button
                  icon={<FaServer />}
                  text={$L('#Home_LocalServer')}
                  theme="default-blurred"
                  onClick={() => ServersConnectService.connectTo(LocalhostServer.address)}
                />
              </Title>
            )}

            <ButtonBar>
              {ServersService.listTypes.map((serverListType) => {
                const eher = serversListsEhers[serverListType];
                if (!eher) {
                  return null;
                }

                const isAllList = eher.to === '/servers';

                return (
                  <Title key={eher.to} title={eher.title}>
                    <LinkButton
                      to={eher.to}
                      text={isAllList ? $L(ServersService.getAllList() ? '#Home_AllList_Link' : '#Home_AllList_Link_Fallback', {
                        count: ServersService.getAllList()?.sequence.length
                      }) : undefined}
                      icon={eher.icon}
                      theme="default-blurred"
                    />
                  </Title>
                );
              })}
            </ButtonBar>
          </Flex>
        </Flex>

        <FlexRestricter vertical>
          <Island className={ui.cls.fullHeight}>
            <Flex fullHeight vertical gap="none">
              <FlexRestricter vertical>
                <VirtualScrollable
                  onScrollUpdate={setScrollOffset}
                  initialScrollOffset={initialScrollOffset}
                  className={ui.cls.fullHeight}
                  itemCount={list.length}
                  itemHeight={UiService.quant * 8}
                  renderItem={(index) => (
                    <IndexedServerListItem
                      list={serversList}
                      index={index}
                    />
                  )}
                />
              </FlexRestricter>
            </Flex>
          </Island>
        </FlexRestricter>
      </Flex>
    </FlexRestricter>
  );
});
