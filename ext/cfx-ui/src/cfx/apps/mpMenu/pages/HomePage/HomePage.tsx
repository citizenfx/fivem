import { Box, Flex, FlexRestricter, Page } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import { FaRetweet } from 'react-icons/fa';
import { FiTwitter } from 'react-icons/fi';

import { InsideNavBar } from 'cfx/apps/mpMenu/parts/NavBar/InsideNavBar';
import { useActivityService } from 'cfx/common/services/activity/activity.service';
import { $L } from 'cfx/common/services/intl/l10n';

import { Continuity } from './Continuity/Continuity';
import { Feed } from './Feed/Feed';
import { Footer } from './Footer/Footer';
import { HomePageNavBarLinks } from './HomePage.links';
import { PlatformStats } from './PlatformStats/PlatformStats';
import { PlatformStatus } from './PlatformStatus/PlatformStatus';
import { TopServersBlock } from './TopServers/TopServers';

import s from './HomePage.module.scss';

export const HomePage = observer(function HomePage() {
  const ActivityService = useActivityService();

  return (
    <Page>
      <InsideNavBar>
        <Flex fullWidth spaceBetween gap="large">
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
                  title={(
                    <>
                      {$L('#Home_Feed_Community_Desc')}
                      <br />
                      <br />
                      <strong>Be aware</strong> that the feed items from the servers you have previously played on will
                      be back in the future.
                      <br />
                      Due to the performance problems we were producing for Mastodon servers, it was disabled.
                    </>
                  )}
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
    </Page>
  );
});
