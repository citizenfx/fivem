import { useService } from "cfx/base/servicesContainer";
import { $L } from "cfx/common/services/intl/l10n";
import { IServersReviewsService } from "cfx/common/services/servers/reviews/serversReviews.service";
import { IServerView } from "cfx/common/services/servers/types";
import { Button } from "cfx/ui/Button/Button";
import { Icons } from "cfx/ui/Icons";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { InfoPanel } from "cfx/ui/InfoPanel/InfoPanel";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";
import { MdOutlineReviews, MdReviews } from "react-icons/md";
import { ServerReview } from "./ServerReview/ServerReview";
import { ServerReviewForm } from "./ServerReviewForm/ServerReviewForm";

export interface ServerReviewsProps {
  server: IServerView,
}

export const ServerReviews = observer(function ServerReviews(props: ServerReviewsProps) {
  const { server } = props;
  if (!server.canReview) {
    return null;
  }

  const ServersReviewsService = useService(IServersReviewsService);

  const serverReviews = ServersReviewsService.getForServer(server.id);

  const nodes = serverReviews.itemsSequence.map((id) => (
    <ServerReview
      key={id}
      review={serverReviews.items[id]}
      serverReviews={serverReviews}
    />
  ));

  return (
    <Flex vertical>
      <Flex centered="axis" gap="small">
        <Text size="small" opacity="75" >
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
        <ServerReviewForm
          server={server}
          serverReviews={serverReviews}
        />
      )}

      {serverReviews.ownReviewApprovePending && (
        <InfoPanel type="success">
          <Flex>
            <MdReviews />
            <span>
              {$L('#Reviews_SubmittedReviewNeedsApproval')}
            </span>
          </Flex>
        </InfoPanel>
      )}

      {!!serverReviews.ownReview && (
        <ServerReview
          disableActions
          review={serverReviews.ownReview}
          serverReviews={serverReviews}
        />
      )}

      {nodes}

      {serverReviews.hasMoreItemsToLoad && (
        <Button
          text="Load more reviews"
          onClick={() => serverReviews.loadMoreItems()}
          disabled={serverReviews.loadingMoreItems}
        />
      )}
    </Flex>
  );
});
