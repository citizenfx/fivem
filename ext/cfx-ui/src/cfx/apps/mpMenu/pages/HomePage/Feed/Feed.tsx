import {
  Icons,
  Island,
  Flex,
  Pad,
  Scrollable,
  Text,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { IActivityItem } from 'cfx/common/services/activity/types';
import { ActivityItem } from 'cfx/ui/ActivityItem/ActivityItem';

export interface FeedProps {
  items: IActivityItem[];

  icon: React.ReactNode;
  label: JSX.Element;
  title: JSX.Element;
}

export const Feed = observer(function Feed(props: FeedProps) {
  const {
    icon,
    label,
    title,
    items,
  } = props;

  const [showFeed, setShowFeed] = React.useState(false);

  React.useLayoutEffect(() => {
    let timer: SetTimeoutReturn | null = setTimeout(() => {
      setShowFeed(true);
      timer = null;
    }, 16);

    return () => {
      if (timer) {
        clearTimeout(timer);
      }
    };
  }, []);

  return (
    <Island widthQ={50}>
      <Flex fullHeight vertical gap="none">
        <Pad>
          <Flex repell>
            <Flex centered>
              <Text size="large" weight="bold" opacity="50">
                {icon}
              </Text>

              <Text size="large" weight="bold" opacity="50">
                {label}
              </Text>
            </Flex>

            <Title fixedOn="left" title={title}>
              <Text size="large" weight="bold" opacity="25">
                {Icons.tipInfo}
              </Text>
            </Title>
          </Flex>
        </Pad>

        <Scrollable>
          {showFeed && (
            <Pad left right bottom>
              <Flex vertical gap="large">
                {items.map((item) => (
                  <ActivityItem key={item.id} item={item} />
                ))}
              </Flex>
            </Pad>
          )}
        </Scrollable>
      </Flex>
    </Island>
  );
});
