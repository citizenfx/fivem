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
import { SERVER_LIST_DESCRIPTORS } from "cfx/common/pages/ServersPage/ListTypeTabs";
import { Title } from "cfx/ui/Title/Title";
import { LinkButton } from "cfx/ui/Button/LinkButton";
import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { Button } from "cfx/ui/Button/Button";
import { ImMagicWand } from 'react-icons/im';
import { useSavedScrollPositionForBackNav } from "cfx/utils/hooks";
import { $L } from "cfx/common/services/intl/l10n";
import s from './HomePage.module.scss';
import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";

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
          <Flex fullWidth stretch gap="large">
            <TopLocaleServer />

            {currentGameNameIs(GameName.FiveM) && (
              <TopStaffPickServer />
            )}
          </Flex>

          <ServersContainer />

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

const TopLocaleServer = observer(function TopLocaleServer() {
  const ServersService = useService(MpMenuServersService);
  if (!ServersService.topLocaleServerId) {
    return null;
  }

  const server = ServersService.getServer(ServersService.topLocaleServerId);
  if (!server) {
    return null;
  }

  return (
    <Flex vertical>
      <Flex centered="axis">
        <Icon size="large" opacity="50">
          <FaMedal />
        </Icon>

        <Text weight="bold" size="large" opacity="50">
          {$L('#Home_PromotedServer_Locale')}
        </Text>
      </Flex>

      <ServerTileItem
        server={server}
      />
    </Flex>
  );
});
const TopStaffPickServer = observer(function TopStaffPickServer() {
  const ServersService = useService(MpMenuServersService);

  const serverId = ServersService.pinnedServersConfig?.noAdServerId;
  if (!serverId) {
    return null;
  }

  const server = ServersService.getServer(serverId);
  if (!server) {
    return null;
  }

  return (
    <Flex vertical>
      <Flex centered="axis">
        <Icon size="large" opacity="50">
          {Icons.serversStaffPickUnstyled}
        </Icon>

        <Text weight="bold" size="large" opacity="50">
          {$L('#Home_PromotedServer_Ad')}
        </Text>
      </Flex>

      <ServerTileItem
        server={server}
      />
    </Flex>
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
        <Flex repell alignToEndAxis>
          <Title title={SERVERS_LIST_TITLE}>
            <Flex centered="axis" gap="normal">
              <Icon size="large" opacity="50">
                <ImMagicWand />
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
                const descriptor = SERVER_LIST_DESCRIPTORS[serverListType];
                if (!descriptor) {
                  return null;
                }

                const isAllList = descriptor.to === '/servers';

                const label = isAllList
                  ? $L(
                    ServersService.totalServersCount
                      ? '#Home_AllList_Link'
                      : '#Home_AllList_Link_Fallback',
                    { count: ServersService.totalServersCount },
                  )
                  : undefined;

                return (
                  <Title key={descriptor.to} title={descriptor.title}>
                    <LinkButton
                      to={descriptor.to}
                      text={label}
                      icon={descriptor.icon}
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
