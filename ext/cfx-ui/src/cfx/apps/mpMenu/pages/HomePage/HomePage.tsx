import { Flex, FlexRestricter, Page } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { InsideNavBar } from 'cfx/apps/mpMenu/parts/NavBar/InsideNavBar';
import { useActivityService } from 'cfx/common/services/activity/activity.service';
import { $L } from 'cfx/common/services/intl/l10n';

import { Continuity } from './Continuity/Continuity';
import { Feed } from './Feed/Feed';
import { Footer } from './Footer/Footer';
import { HomePageNavBarLinks } from './HomePage.links';
import { PlatformStats } from './PlatformStats/PlatformStats';
import { TopServersBlock } from './TopServers/TopServers';
import { PlatformStatus } from '../../parts/PlatformStatus/PlatformStatus';

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

        <Flex vertical fullHeight gap="xlarge">
          <FlexRestricter vertical>
            <Flex fullHeight>
              <Feed
                items={ActivityService.officialItems}
                label="X.com Feed"
                title={$L('#Home_Feed_Official_Desc')}
              />
            </Flex>
          </FlexRestricter>
        </Flex>
      </Flex>
    </Page>
  );
});
