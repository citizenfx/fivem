import { useL10nPlural } from "cfx/common/services/intl/l10n";
import { PluralKeys, PluralRule } from "cfx/common/services/intl/types";
import { IServerReviewItemReactions, ServerReviewReaction } from "cfx/common/services/servers/reviews/types";
import { Button } from "cfx/ui/Button/Button";
import { ButtonBar } from "cfx/ui/Button/ButtonBar";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { Title } from "cfx/ui/Title/Title";
import { getValue, ValueOrGetter } from "cfx/utils/getValue";
import { observer } from "mobx-react-lite";
import { BsEmojiLaughing, BsHandThumbsDown, BsHandThumbsUp } from "react-icons/bs";

const REACTIONS_SEQUENCE = [
  ServerReviewReaction.Helpful,
  ServerReviewReaction.Unhelpful,
  ServerReviewReaction.Funny,
];

interface ServerReviewReactionsProps {
  reactions: IServerReviewItemReactions,

  disabled?: boolean,
}
export const ServerReviewReactions = observer(function ServerReviewReactions(props: ServerReviewReactionsProps) {
  const {
    reactions,
    disabled = false,
  } = props;

  const isHelpful = reactions.count[ServerReviewReaction.Helpful] >= reactions.count[ServerReviewReaction.Unhelpful];
  const isHelpfulCount = isHelpful
    ? reactions.count[ServerReviewReaction.Helpful]
    : reactions.count[ServerReviewReaction.Unhelpful];

  const reactionSentimentNode = useL10nPlural(
    isHelpfulCount,
    isHelpful
      ? helpfullKeys
      : unhelpfullKeys,
  );

  const nodes = REACTIONS_SEQUENCE.map((reaction) => {
    const { icon, title } = reactionButtons[reaction];

    return (
      <Title key={reaction} title={getValue(title)}>
        <Button
          size="small"
          icon={icon}
          theme={reactions.hasReaction(reaction) ? 'primary' : 'default'}
          text={reactions.count[reaction]}
          disabled={disabled || !reactions.canReact(reaction)}
          onClick={() => reactions.react(reaction)}
        />
      </Title>
    );
  });

  return (
    <Flex centered="axis">
      <ButtonBar>
        {nodes}
      </ButtonBar>

      <Text size="small" opacity="75">
        {reactionSentimentNode}
      </Text>
    </Flex>
  );
});

const reactionButtons: Record<ServerReviewReaction, { icon: React.ReactNode, title: ValueOrGetter<React.ReactNode> }> = {
  [ServerReviewReaction.Funny]: {
    icon: <BsEmojiLaughing />,
    title: 'Funny review',
  },
  [ServerReviewReaction.Helpful]: {
    icon: <BsHandThumbsUp />,
    title: 'This review helped me!',
  },
  [ServerReviewReaction.Unhelpful]: {
    icon: <BsHandThumbsDown />,
    title: 'Unhelpful review :\\',
  },
};

const unhelpfullKeys: PluralKeys = {
  [PluralRule.One]: '@Review_UserUnhelpful',
  [PluralRule.Other]: '@Review_UsersUnhelpful',
};
const helpfullKeys: PluralKeys = {
  [PluralRule.One]: '@Review_UserHelpful',
  [PluralRule.Other]: '@Review_UsersHelpful',
};
const funnyKeys: PluralKeys = {
  [PluralRule.One]: '@Review_UserFunny',
  [PluralRule.Other]: '@Review_UsersFunny',
};
