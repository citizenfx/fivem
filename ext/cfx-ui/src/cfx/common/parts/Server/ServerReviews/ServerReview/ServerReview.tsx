import React from "react";
import format from "date-fns/format";
import { IServerReviewItem, IServerReviews } from "cfx/common/services/servers/reviews/types";
import { Avatar } from "cfx/ui/Avatar/Avatar";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text, TextBlock } from "cfx/ui/Text/Text";
import { Title } from "cfx/ui/Title/Title";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { observer } from "mobx-react-lite";
import { ServerReviewSentimentIcon } from "./ServerReviewSentimentIcon";
import { Separator } from "cfx/ui/Separator/Separator";
import { ServerReviewReactions } from "./ServerReviewReactions";
import { ServerReviewReport } from "./ServerReviewReport";
import s from './ServerReview.module.scss';

export interface ServerReviewProps {
  review: IServerReviewItem,
  serverReviews: IServerReviews,

  disableActions?: boolean,
}

export const ServerReview = observer(function ServerReview(props: ServerReviewProps) {
  const {
    review,
    serverReviews,

    disableActions = false,
  } = props;

  const playtimeRaw = serverReviews.playtimes[review.authorCfxId];
  const playtime = playtimeRaw?.formattedSeconds || 'loading';

  const showReactions = !!review.reactions;
  const showReport = !disableActions && Number(review.report?.options?.length) > 0;

  return (
    <ServerReviewLayout
      avatar={<Avatar url={review.authorAvatarURL} />}
      authorName={
        <Title fixedOn="bottom-left" title={review.authorName}>
          <Text truncated typographic>
            {review.authorName}
          </Text>
        </Title>
      }
      playtime={
        <Title title="Time user spent on server">
          <TextBlock size="small" opacity="75">
            {playtime}
          </TextBlock>
        </Title>
      }
      postedAt={
        <Text size="small" opacity="25">
          Posted: {format(review.createdAt, 'd MMM y')}
        </Text>
      }
      title={
        <>
          <ServerReviewSentimentIcon sentiment={review.sentiment} />

          <Text size="large">
            {review.title}
          </Text>
        </>
      }
      content={
        <TextBlock typographic className={s.content}>
          {review.content}
        </TextBlock>
      }
      reactions={
        showReactions && (
          <ServerReviewReactions
            disabled={disableActions}
            reactions={review.reactions!}
          />
        )
      }
      report={
        showReport && (
          <ServerReviewReport
            report={review.report!}
            className={s['show-on-hover']}
          />
        )
      }
    />
  );
});

type LayoutProps<T extends string> = { [key in T]: React.ReactNode };

function ServerReviewLayout(props: LayoutProps<'avatar' | 'authorName' | 'playtime' | 'postedAt' | 'title' | 'content' | 'reactions' | 'report'>) {
  const {
    avatar,
    authorName,
    playtime,
    postedAt,
    title,
    content,
    reactions,
    report,
  } = props;

  return (
    <Box className={s.root}>
      <Pad>
        <Flex>
          <Box width={30} noShrink noOverflow>
            <Flex vertical repell fullHeight>
              <Flex>
                {avatar}

                <Box grow>
                  <Flex vertical gap="small">
                    {authorName}

                    {playtime}
                  </Flex>
                </Box>
              </Flex>

              {postedAt}
            </Flex>
          </Box>

          <Box grow>
            <Flex vertical>
              <Flex centered="axis">
                {title}
              </Flex>

              {content}

              {(reactions || report) && (
                <>
                  <Separator thin />

                  <Flex repell centered>
                    {reactions}

                    {report}
                  </Flex>
                </>
              )}
            </Flex>
          </Box>
        </Flex>
      </Pad>
    </Box>
  );
}
