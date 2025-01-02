import { Flex, FlexRestricter, Page } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
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
            <Flex fullHeight>
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
    </Page>
  );
});
