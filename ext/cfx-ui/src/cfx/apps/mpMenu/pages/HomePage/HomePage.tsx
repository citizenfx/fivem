import { Icons } from "cfx/ui/Icons";
import { Page } from "cfx/ui/Layout/Page/Page";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { InsideNavBar } from "cfx/apps/mpMenu/parts/NavBar/InsideNavBar";
import { observer } from "mobx-react-lite";
import { HomePageNavBarLinks } from "./HomePage.links";
import { Text } from "cfx/ui/Text/Text";
import { Feed } from "./Feed/Feed";
import { Box } from "cfx/ui/Layout/Box/Box";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { PlatformStatus } from "./PlatformStatus/PlatformStatus";
import { useActivityService } from "cfx/common/services/activity/activity.service";
import { FaRetweet } from "react-icons/fa";
import { FiTwitter } from "react-icons/fi";
import { Icon } from "cfx/ui/Icon/Icon";
import { $L } from "cfx/common/services/intl/l10n";
import { PlatformStats } from "./PlatformStats/PlatformStats";
import { Carousel } from "./Carousel/Carousel";
import { CountryFlag } from "cfx/ui/CountryFlag/CountryFlag";
import { useIntlService } from "cfx/common/services/intl/intl.service";
import { ExtraLinkyTiles } from "./ExtraLinkyTiles/ExtraLinkyTiles";
import { LastConnectedTile } from "./LastConnectedTile/LastConnectedTile";
import { FeaturedServerTile } from "./FeaturedServerTile/FeaturedServerTile";
import { AuxiliaryGameModes } from "./AuxiliaryGameModes/AuxiliaryGameModes";
import { ServersNavBar } from "./ServersNavBar/ServersNavBar";
import s from './HomePage.module.scss';
import { ui } from "cfx/ui/ui";

export const HomePage = observer(function HomePage() {
  const ActivityService = useActivityService();
  const IntlService = useIntlService();

  return (
    <Page>
      <InsideNavBar>
        <Flex fullWidth spaceBetween gap="large">
          <ServersNavBar />

          <PlatformStatus />

          <HomePageNavBarLinks />

          <PlatformStats />
        </Flex>
      </InsideNavBar>

      <Flex fullHeight gap="xlarge">
        <FlexRestricter>
          <Flex fullWidth fullHeight vertical repell gap="none">
            <Box height="60%">
              <Flex vertical fullHeight>
                <Flex centered="axis">
                  <Icon size="large" opacity="50">
                    {Icons.serverBoostUnstyled}
                  </Icon>

                  <CountryFlag country={IntlService.systemLocaleCountry} locale={IntlService.systemLocale} />

                  <Text size="large" opacity="50" weight="bold">
                    Top servers
                  </Text>
                </Flex>

                <Carousel />
              </Flex>
            </Box>

            <Box>
              <Flex fullWidth fullHeight alignToEndAxis gap="large">
                <Box width="46%">
                  <Flex vertical gap="large">
                    <LastConnectedTile />

                    <FeaturedServerTile />

                    <AuxiliaryGameModes />
                  </Flex>
                </Box>

                <div />

                <Box width="54%" height="100%">
                  <ExtraLinkyTiles />
                </Box>
              </Flex>
            </Box>
          </Flex>
        </FlexRestricter>

        <Flex vertical fullHeight gap="large">
          <FlexRestricter vertical>
            <Box height="100%" width="100%">
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
            </Box>
          </FlexRestricter>
        </Flex>
      </Flex>
    </Page >
  );
});
