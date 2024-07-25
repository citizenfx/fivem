import {
  Button,
  Indicator,
  InfoPanel,
  Flex,
  Scrollable,
  Text,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsEmojiFrownFill } from 'react-icons/bs';
import { IoNewspaperSharp } from 'react-icons/io5';

import { ActiveActivityPubFeed } from 'cfx/common/services/activity/ActiveActivityPubFeed';
import { useActivityService } from 'cfx/common/services/activity/activity.service';
import { $L } from 'cfx/common/services/intl/l10n';
import { ActivityItem } from 'cfx/ui/ActivityItem/ActivityItem';
import { Flyout } from 'cfx/ui/Flyout/Flyout';

import s from './ServerActivityFeed.module.scss';

export interface ServerActivityFeedProps {
  pub: string;
}

export const ServerActivityFeed = observer(function ServerActivityFeed(props: ServerActivityFeedProps) {
  const {
    pub,
  } = props;

  const feed = useActivityService().getActiveActivityPubFeed(pub);

  const [showAll, setShowAll] = React.useState(false);

  if (feed.loadingInit) {
    return (
      <Flex vertical>
        <Header id={feed.id} />

        <Indicator />
      </Flex>
    );
  }

  if (feed.hasInitError) {
    return (
      <Flex vertical>
        <Header id={feed.id} />

        <InfoPanel>
          <Flex vertical centered>
            <Text size="xxlarge">
              <BsEmojiFrownFill />
            </Text>

            <Text size="large">{$L('#ServerDetail_Feed_LoadError', { feed: feed.id })}</Text>

            {feed.initError && (
              <Text opacity="75">
                {$L('#ServerDetail_Feed_LoadError_ServerReported')} {feed.initError}
              </Text>
            )}
          </Flex>
        </InfoPanel>
      </Flex>
    );
  }

  if (feed.itemsTotal === 0) {
    return (
      <Flex vertical>
        <Header id={feed.id} />

        <InfoPanel>
          <Flex vertical centered>
            <Text size="xxlarge">
              <BsEmojiFrownFill />
            </Text>

            <Text size="large">{$L('#ServerDetail_Feed_Empty', { feed: feed.id })}</Text>
          </Flex>
        </InfoPanel>
      </Flex>
    );
  }

  const mainItems = feed.items.slice(0, 3);

  const nodes = mainItems.map((item) => (
    <div key={item.id} className={s.item}>
      <ActivityItem item={item} />
    </div>
  ));

  const hasMoreItems = mainItems.length < feed.itemsTotal;

  return (
    <>
      {showAll && (
        <ServerActivityFeedFlyout feed={feed} onClose={() => setShowAll(false)} />
      )}

      <Flex fullWidth vertical>
        <Header id={feed.id} />

        <Flex fullWidth gap="normal">
          {nodes}

          {hasMoreItems && (
            <div className={s.morer}>
              <Button text={$L('#ServerDetail_Feed_ShowMore')} onClick={() => setShowAll(true)} />
            </div>
          )}
        </Flex>
      </Flex>
    </>
  );
});

function Header({
  id,
}: { id: string }) {
  return (
    <Flex repell>
      <Flex centered gap="small">
        <Text size="small" opacity="75">
          <IoNewspaperSharp />
        </Text>

        <Text uppercase size="small" opacity="75">
          {id}
        </Text>
      </Flex>
    </Flex>
  );
}

const ServerActivityFeedFlyout = observer(function ServerActivityFeedFlyout({
  feed,
  onClose,
}: {
  feed: ActiveActivityPubFeed;
  onClose(): void;
}) {
  React.useEffect(() => {
    feed.loadAll();
  }, [feed]);

  const nodes = feed.items.map((item) => (
    <ActivityItem key={item.id} item={item} />
  ));

  return (
    <Flyout size="xsmall" onClose={onClose}>
      <Flex fullWidth fullHeight vertical gap="xlarge">
        <Flyout.Header onClose={onClose}>
          <IoNewspaperSharp />

          <Text truncated typographic size="xxlarge">
            {feed.id}
          </Text>
        </Flyout.Header>

        <Scrollable>
          <Flex fullWidth vertical gap="large">
            {nodes}
          </Flex>
        </Scrollable>
      </Flex>
    </Flyout>
  );
});
