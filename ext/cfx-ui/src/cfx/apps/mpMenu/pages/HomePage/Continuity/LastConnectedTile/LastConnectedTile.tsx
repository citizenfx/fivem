import { Interactive, Text } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { useStreamerMode } from 'cfx/apps/mpMenu/services/convars/convars.service';
import { useHomeScreenServerList } from 'cfx/apps/mpMenu/services/servers/list/HomeScreenServerList.service';
import { ServerTileItem } from 'cfx/common/parts/Server/ServerTileItem/ServerTileItem';
import { ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { useOpenFlag } from 'cfx/utils/hooks';

import s from './LastConnectedTile.module.scss';

export const LastConnectedTile = observer(function LastConnectedTile() {
  const [warningHidden, hide] = useOpenFlag(false);

  const streamerMode = useStreamerMode();
  const showStreamerWarning = !warningHidden && streamerMode;

  const server = useLastHistoryServer();

  if (!server) {
    return null;
  }

  const label = (
    <Text size="normal" opacity="50" weight="thin">
      {$L('#Home_LastConnectedServer')}
    </Text>
  );

  return (
    <div className={s.root}>
      <ServerTileItem
        placeControlsBelow
        hideBanner
        hideBoost
        label={label}
        server={server}
        elementPlacement={ElementPlacements.LastConnected}
      />

      {showStreamerWarning && (
        <Interactive onClick={hide} className={s.warning}>
          <Text color="inherit" size="large">
            {$L('#Home_LastConnectedServer_PrivacyCover_Header')}
          </Text>

          <Text color="inherit">{$L('#Home_LastConnectedServer_PrivacyCover_Reason')}</Text>

          <div />

          <Text color="inherit" weight="bold">
            {$L('#Home_LastConnectedServer_PrivacyCover_ClickToShow')}
          </Text>
        </Interactive>
      )}
    </div>
  );
});

export function useLastHistoryServer() {
  const HomeScreenServerList = useHomeScreenServerList();

  return HomeScreenServerList.lastConnectedServer;
}
