import { Icons } from '@cfx-dev/ui-components';
import React from 'react';

import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { ElementPlacements, EventActionNames, isFeaturedElementPlacement } from 'cfx/common/services/analytics/types';
import { IServerView } from 'cfx/common/services/servers/types';
import { LinkButton } from 'cfx/ui/Button/LinkButton';

export interface ServerReportButtonProps {
  server: IServerView;
  elementPlacement?: ElementPlacements;
}

export function ServerReportButton(props: ServerReportButtonProps) {
  const {
    server,
    elementPlacement = ElementPlacements.Unknown,
  } = props;

  const formURL = React.useMemo(() => {
    const baseURL = 'https://support.cfx.re/hc/en-us/requests/new?ticket_form_id=13998414161564';
    const serverIDField = 'tf_14094297286300';
    const serverIDValue = encodeURIComponent(server.id);

    const serverNameField = 'tf_14094396104988';
    const serverNameValue = encodeURIComponent(server.hostname);

    return `${baseURL}&${serverIDField}=${serverIDValue}&${serverNameField}=${serverNameValue}`;
  }, [server]);

  const eventHandler = useEventHandler();

  const handleClick = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.ReportCTA,
      properties: {
        element_placement: elementPlacement,
        server_id: server.id,
        server_name: server.projectName || server.hostname,
        server_type: isFeaturedElementPlacement(elementPlacement)
          ? 'featured'
          : undefined,
      },
    });
  }, [eventHandler, server, elementPlacement]);

  return (
    <div>
      <LinkButton
        to={formURL}
        text="Report Server"
        theme="transparent"
        size="small"
        icon={Icons.warning}
        onClick={handleClick}
        isHrefProp
      />
    </div>
  );
}
