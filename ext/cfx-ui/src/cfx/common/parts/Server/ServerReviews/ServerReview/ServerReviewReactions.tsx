import {
  Button,
  ButtonBar,
  Flex,
  Text,
  Title,
  getValue,
  ValueOrGetter,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import { BsEmojiLaughing, BsHandThumbsDown, BsHandThumbsUp } from 'react-icons/bs';

import { $L, useL10nPlural } from 'cfx/common/services/intl/l10n';
import { PluralKeys, PluralRule } from 'cfx/common/services/intl/types';
import { IServerReviewItemReactions, ServerReviewReaction } from 'cfx/common/services/servers/reviews/types';

const REACTIONS_SEQUENCE = [ServerReviewReaction.Helpful, ServerReviewReaction.Unhelpful, ServerReviewReaction.Funny];

interface ServerReviewReactionsProps {
  reactions: IServerReviewItemReactions;

  disabled?: boolean;
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

  const reactionSentimentNode = useL10nPlural(isHelpfulCount, isHelpful
    ? helpfullKeys
    : unhelpfullKeys);

  const nodes = REACTIONS_SEQUENCE.map((reaction) => {
    const {
      icon,
      title,
    } = reactionButtons[reaction];

    return (
      <Title key={reaction} title={getValue(title)}>
        <Button
          size="small"
          icon={icon}
          theme={reactions.hasReaction(reaction)
            ? 'primary'
            : 'default'}
          text={reactions.count[reaction]}
          disabled={disabled || !reactions.canReact(reaction)}
          onClick={() => reactions.react(reaction)}
        />
      </Title>
    );
  });

  return (
    <Flex centered="axis">
      <ButtonBar>{nodes}</ButtonBar>

      <Text size="small" opacity="75">
        {reactionSentimentNode}
      </Text>
    </Flex>
  );
});

interface ReactionButtonItem {
  icon: React.ReactNode;
  title: ValueOrGetter<React.ReactNode>;
}

const reactionButtons: Record<ServerReviewReaction, ReactionButtonItem> = {
  [ServerReviewReaction.Funny]: {
    icon: <BsEmojiLaughing />,
    title: $L('#Review_Reaction_Funny'),
  },
  [ServerReviewReaction.Helpful]: {
    icon: <BsHandThumbsUp />,
    title: $L('#Review_Reaction_Helpful'),
  },
  [ServerReviewReaction.Unhelpful]: {
    icon: <BsHandThumbsDown />,
    title: $L('#Review_Reaction_Unhelpful'),
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
// const funnyKeys: PluralKeys = {
//   [PluralRule.One]: '@Review_UserFunny',
//   [PluralRule.Other]: '@Review_UsersFunny',
// };
