import { Page } from "cfx/ui/Layout/Page/Page";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { InsideNavBar } from "cfx/apps/mpMenu/parts/NavBar/InsideNavBar";
import { observer } from "mobx-react-lite";
import { HomePageNavBarLinks } from "./HomePage.links";
import { Feed } from "./Feed/Feed";
import { Box } from "cfx/ui/Layout/Box/Box";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { PlatformStatus } from "./PlatformStatus/PlatformStatus";
import { useActivityService } from "cfx/common/services/activity/activity.service";
import { FaRetweet } from "react-icons/fa";
import { FiTwitter } from "react-icons/fi";
import { $L } from "cfx/common/services/intl/l10n";
import { PlatformStats } from "./PlatformStats/PlatformStats";
import { TopServersBlock } from "./TopServers/TopServers";
import { Continuity } from "./Continuity/Continuity";
import { Footer } from "./Footer/Footer";
import { BetaSign } from "./BetaSign/BetaSign";
import s from './HomePage.module.scss';

export const HomePage = observer(function HomePage() {
  const ActivityService = useActivityService();

  return (
    <Page>
      <InsideNavBar>
        <Flex fullWidth spaceBetween gap="large">
          <BetaSign />

          <HomePageNavBarLinks />

          <PlatformStatus />

          <PlatformStats />
        </Flex>
      </InsideNavBar>

      <Flex fullHeight gap="xlarge">
        <FlexRestricter>
          <Flex fullWidth fullHeight vertical repell gap="xlarge">
            <Continuity />

            <TopServersBlock />

            <Footer />
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
