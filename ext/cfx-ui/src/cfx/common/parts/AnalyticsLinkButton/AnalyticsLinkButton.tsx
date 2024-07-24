import { noop, isExternalUrl } from '@cfx-dev/ui-components';
import React from 'react';

import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';
import { LinkButtonProps, LinkButton } from 'cfx/ui/Button/LinkButton';

type Props = LinkButtonProps & {
  elementPlacement?: ElementPlacements;
};

export const AnalyticsLinkButton = React.forwardRef(function AnalyticsLinkButton(
  props: Props,
  ref: React.Ref<HTMLAnchorElement>,
) {
  const {
    onClick = noop,
    elementPlacement = ElementPlacements.Unknown,
    ...restProps
  } = props;
  const {
    text,
    to,
  } = props;
  const eventHandler = useEventHandler();
  const isExternalLink = React.useMemo(() => isExternalUrl(to), [to]);

  const handleClick = React.useCallback(() => {
    onClick();

    if (isExternalLink) {
      eventHandler({
        action: EventActionNames.SiteLinkClick,
        properties: {
          element_placement: elementPlacement,
          text: text?.toString() || '',
          link_url: to,
        },
      });
    }
  }, [onClick, text, to, elementPlacement, isExternalLink, eventHandler]);

  return (
    <LinkButton {...restProps} onClick={handleClick} ref={ref} />
  );
});
