import React from "react";
import { ActivityItem } from "cfx/ui/ActivityItem/ActivityItem";
import { Icons } from "cfx/ui/Icons";
import { Island } from "cfx/ui/Island/Island";
import { Text } from "cfx/ui/Text/Text";
import { Title } from "cfx/ui/Title/Title";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Scrollable } from "cfx/ui/Layout/Scrollable/Scrollable";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { IActivityItem } from "cfx/common/services/activity/types";
import { observer } from "mobx-react-lite";

export interface FeedProps {
  items: IActivityItem[],

  icon: React.ReactNode,
  label: JSX.Element,
  title: JSX.Element,
}

export const Feed = observer(function Feed(props: FeedProps) {
  const {
    icon,
    label,
    title,
    items,
  } = props;

  const [showFeed, setShowFeed] = React.useState(false);

  React.useEffect(() => {
    let timer: SetTimeoutReturn | null = setTimeout(() => {
      setShowFeed(true);
      timer = null;
    }, 50);

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
})
