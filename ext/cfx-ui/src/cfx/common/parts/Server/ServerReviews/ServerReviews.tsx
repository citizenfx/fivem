/* eslint-disable react/no-unescaped-entities */
import {
  Button,
  Icons,
  Indicator,
  InfoPanel,
  Flex,
  Pad,
  Text,
  TextBlock,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import { MdOutlineReviews, MdReviews } from 'react-icons/md';

import { useService } from 'cfx/base/servicesContainer';
import { $L } from 'cfx/common/services/intl/l10n';
import { IServersReviewsService } from 'cfx/common/services/servers/reviews/serversReviews.service';
import { IServerView } from 'cfx/common/services/servers/types';

import { ServerReview } from './ServerReview/ServerReview';

export interface ServerReviewsProps {
  server: IServerView;
}

export const ServerReviews = observer(function ServerReviews(props: ServerReviewsProps) {
  const {
    server,
  } = props;

  const ServersReviewsService = useService(IServersReviewsService);

  if (!server.canReview) {
    return null;
  }

  const serverReviews = ServersReviewsService.getForServer(server.id);

  const nodes = serverReviews.itemsSequence.map((id) => (
    <ServerReview key={id} review={serverReviews.items[id]} serverReviews={serverReviews} />
  ));

  return (
    <Flex vertical>
      <Flex centered="axis" gap="small">
        <Text size="small" opacity="75">
          <MdOutlineReviews />
        </Text>

        <Text uppercase size="small" opacity="75">
          {$L('#Reviews')}
          {/* Would be nice, but Discourse reports nonsense */}
          {/* ({reviews.totalItemsCount}) */}
        </Text>

        {serverReviews.initialLoading && (
          <Indicator />
        )}
      </Flex>

      {serverReviews.canSubmitReview && (
        <Pad top bottom>
          <InfoPanel type="warning" icon={Icons.statusLevelMinor}>
            <Flex vertical>
              <Text size="large" weight="bold">
                Posting reviews is temporarily disabled
              </Text>
              <TextBlock typographic opacity="75">
                Due to moderation capacity issues, we've had to temporarily disable server review submissions.
                <br />
                We'll try to resolve this as soon as possible, hang in there!
              </TextBlock>
            </Flex>
          </InfoPanel>
        </Pad>
      )}

      {/* {serverReviews.canSubmitReview && (
        <ServerReviewForm
          server={server}
          serverReviews={serverReviews}
        />
      )} */}

      {serverReviews.ownReviewApprovePending && (
        <InfoPanel type="success">
          <Flex>
            <MdReviews />
            <span>{$L('#Reviews_SubmittedReviewNeedsApproval')}</span>
          </Flex>
        </InfoPanel>
      )}

      {!!serverReviews.ownReview && (
        <ServerReview disableActions review={serverReviews.ownReview} serverReviews={serverReviews} />
      )}

      {nodes}

      {serverReviews.hasMoreItemsToLoad && (
        <Button
          text={$L('#Reviews_LoadMore')}
          onClick={() => serverReviews.loadMoreItems()}
          disabled={serverReviews.loadingMoreItems}
        />
      )}
    </Flex>
  );
});
