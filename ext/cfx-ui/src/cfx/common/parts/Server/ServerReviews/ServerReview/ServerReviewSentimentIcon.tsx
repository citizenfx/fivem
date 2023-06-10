import { ServerReviewSentiment } from "cfx/common/services/servers/reviews/types";
import { Icon } from "cfx/ui/Icon/Icon";
import { ui } from "cfx/ui/ui";
import { BsHandThumbsDown, BsHandThumbsUp } from "react-icons/bs";

const REVIEW_SENTIMENT_ICON: Record<ServerReviewSentiment, React.ReactNode> = {
  [ServerReviewSentiment.Undecided]: null,
  [ServerReviewSentiment.Recommend]: (
    <BsHandThumbsUp
      style={{
        color: ui.color('success'),
        filter: `drop-shadow(0 0 2px ${ui.color('success', 300)})`,
      }}
    />
  ),
  [ServerReviewSentiment.NotRecommend]: (
    <BsHandThumbsDown
      style={{
        color: ui.color('error'),
        filter: `drop-shadow(0 0 2px ${ui.color('error', 300)})`,
      }}
    />
  ),
};

export interface ServerReviewSentimentIconProps {
  sentiment: ServerReviewSentiment,
}

export function ServerReviewSentimentIcon({ sentiment }: ServerReviewSentimentIconProps) {
  return (
    <Icon size="xxlarge">
      {REVIEW_SENTIMENT_ICON[sentiment]}
    </Icon>
  );
}
