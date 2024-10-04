import { Button, Title, noop } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { playSfx, Sfx } from 'cfx/apps/mpMenu/utils/sfx';
import { CurrentGameBuild, CurrentGamePureLevel } from 'cfx/base/gameRuntime';
import { hasConnectEndpoints, isServerEOL } from 'cfx/base/serverUtils';
import { useServiceOptional } from 'cfx/base/servicesContainer';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements, isFeaturedElementPlacement } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { isServerOffline } from 'cfx/common/services/servers/helpers';
import { IServersConnectService } from 'cfx/common/services/servers/serversConnect.service';
import { IServerView } from 'cfx/common/services/servers/types';
import { timeout } from 'cfx/utils/async';
import { stopPropagation } from 'cfx/utils/domEvents';
import { SingleEventListenerDisposer } from 'cfx/utils/singleEventEmitter';

// Random number that feels enough to wait for fail
const CONNECTION_TIMEOUT_TIME = 2500;
export interface ServerConnectButtonProps {
  server: IServerView;

  size?: React.ComponentProps<typeof Button>['size'];
  theme?: React.ComponentProps<typeof Button>['theme'];
  elementPlacement?: ElementPlacements;
}

export const ServerConnectButton = observer(function ServerConnectButton(props: ServerConnectButtonProps) {
  const {
    server,
    size = 'large',
    theme = 'primary',
    elementPlacement = ElementPlacements.Unknown,
  } = props;

  const ServersConnectService = useServiceOptional(IServersConnectService);
  const eventHandler = useEventHandler();
  const stopConnectionListnerRef = React.useRef<SingleEventListenerDisposer | null>(null);

  let title: React.ReactNode;
  let canConnect = hasConnectEndpoints(server);

  if (!server.historicalAddress) {
    switch (true) {
      case isServerEOL(server): {
        title = $L('#ServerDetail_EOLDisable');
        canConnect = false;
        break;
      }

      case !!server.private: {
        title = $L('#ServerDetail_PrivateDisable');
        break;
      }

      case isServerOffline(server): {
        title = $L('#ServerDetail_OfflineDisable');
        break;
      }
    }

    if (canConnect) {
      if (server.enforceGameBuild || server.pureLevel) {
        const hasKnownGameBuild = CurrentGameBuild !== '-1';
        const hasKnownGamePureLevel = CurrentGamePureLevel !== '-1';

        const shouldSwitchGameBuild = hasKnownGameBuild
          && server.enforceGameBuild
          && CurrentGameBuild !== server.enforceGameBuild;
        const shouldSwitchPureLevel = hasKnownGamePureLevel
          && server.pureLevel
          && CurrentGamePureLevel !== server.pureLevel;

        if (shouldSwitchGameBuild && shouldSwitchPureLevel) {
          title = $L('#DirectConnect_SwitchBuildPureLevelAndConnect');
        } else if (shouldSwitchGameBuild) {
          title = $L('#DirectConnect_SwitchBuildAndConnect');
        } else if (shouldSwitchPureLevel) {
          title = $L('#DirectConnect_SwitchPureLevelAndConnect');
        }
      }
    }
  }

  const disabled = !ServersConnectService?.canConnect || !canConnect;

  const handleConnectFailed = React.useCallback(
    (text: string) => {
      eventHandler({
        action: EventActionNames.ServerJoinFailed,
        properties: {
          element_placement: elementPlacement,
          server_id: server.id,
          server_name: server.projectName || server.hostname,
          server_type: isFeaturedElementPlacement(elementPlacement)
            ? 'featured'
            : undefined,
          text,
          link_url: '/',
        },
      });
    },
    [eventHandler, elementPlacement, server],
  );

  const handleClick = ServersConnectService
    ? () => {
        if (__CFXUI_USE_SOUNDS__) {
          playSfx(Sfx.Connect);
        }

        if (stopConnectionListnerRef.current) {
          stopConnectionListnerRef.current();
          stopConnectionListnerRef.current = null;
        }

        eventHandler({
          action: EventActionNames.ServerConnectCTA,
          properties: {
            element_placement: elementPlacement,
            server_id: server.id,
            server_name: server.projectName || server.hostname,
            server_type: isFeaturedElementPlacement(elementPlacement)
              ? 'featured'
              : undefined,
            text: '#DirectConnect_Connect',
            link_url: '/',
          },
        });

        stopConnectionListnerRef.current = ServersConnectService.connectFailed.addListener(handleConnectFailed);
        timeout(CONNECTION_TIMEOUT_TIME).finally(() => {
          if (stopConnectionListnerRef.current === null) {
            return;
          }

          stopConnectionListnerRef.current();
          stopConnectionListnerRef.current = null;
        });

        ServersConnectService.connectTo(server);
      }
    : noop;

  return (
    <Title fixedOn="bottom-left" title={title}>
      <Button
        size={size}
        theme={theme}
        disabled={disabled}
        text={$L('#DirectConnect_Connect')}
        onClick={stopPropagation(handleClick)}
      />
    </Title>
  );
});
