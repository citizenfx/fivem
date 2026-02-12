import { Flex, Icons, Interactive, Text, clsx } from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { FaServer } from 'react-icons/fa';
import { Link } from 'react-router-dom';

import { useStreamerMode } from 'cfx/apps/mpMenu/services/convars/convars.service';
import { MpMenuLocalhostServerService } from 'cfx/apps/mpMenu/services/servers/localhostServer.mpMenu';
import { useService } from 'cfx/base/servicesContainer';
import { SERVER_LIST_DESCRIPTORS } from 'cfx/common/pages/ServersPage/ListTypeTabs';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { ServersListType } from 'cfx/common/services/servers/lists/types';
import { useServersService } from 'cfx/common/services/servers/servers.service';
import { IServersConnectService } from 'cfx/common/services/servers/serversConnect.service';

import { LastConnectedTile, useLastHistoryServer } from './LastConnectedTile/LastConnectedTile';

import s from './Continuity.module.scss';

export const Continuity = observer(function Continuity() {
  const hasLastConnectedServer = Boolean(useLastHistoryServer());
  const hasLocalhostServer = useHasLocalhostServer();

  const rootClassName = clsx(s.root, {
    [s.withLast]: hasLastConnectedServer,
    [s.withLocalhost]: hasLocalhostServer,
  });

  return (
    <div className={rootClassName}>
      <PlayTile />

      {hasLocalhostServer && (
        <LocalhostTile />
      )}

      {hasLastConnectedServer && (
        <div className={clsx(s.slot, s.slotLast)}>
          <LastConnectedTile />
        </div>
      )}

      <ListTile serversListType={ServersListType.Favorites} />
      <ListTile serversListType={ServersListType.History} />
    </div>
  );
});

type ListTileProps = {
  serversListType: ServersListType;
};

const ListTile = observer(function ListTile({
  serversListType,
}: ListTileProps) {
  const ServersService = useServersService();
  const eventHandler = useEventHandler();

  const list = ServersService.getList(serversListType);

  if (!list) {
    return null;
  }

  const descriptor = SERVER_LIST_DESCRIPTORS[serversListType];

  const tileClassName = clsx(s.tile, {
    [s.tileFavorites]: serversListType === ServersListType.Favorites,
    [s.tileHistory]: serversListType === ServersListType.History,
  });

  // eslint-disable-next-line react-hooks/rules-of-hooks
  const handleClick = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.AccountInfoCTA,
      properties: {
        element_placement: ElementPlacements.Continuity,
        text: descriptor.titleKey,
        link_url: descriptor.to,
      },
    });
  }, [eventHandler, descriptor]);

  return (
    <Link to={descriptor.to} onClick={handleClick} className={tileClassName}>
      <div className={s.icon}>{descriptor.icon}</div>

      <div className={s.title}>{$L(descriptor.titleKey)}</div>
    </Link>
  );
});

const PlayTile = observer(function PlayTile() {
  const ServersService = useServersService();
  const eventHandler = useEventHandler();

  const list = ServersService.getList(ServersListType.All);

  if (!list) {
    return null;
  }

  const descriptor = SERVER_LIST_DESCRIPTORS[ServersListType.All];

  const tileClassName = clsx(s.tile, s.tilePlay);

  // eslint-disable-next-line react-hooks/rules-of-hooks
  const handlePlayClick = React.useCallback(() => {
    eventHandler({
      action: EventActionNames.PlayCTA,
      properties: {
        element_placement: ElementPlacements.Continuity,
        text: '#BottomNav_Play',
        link_url: descriptor.to,
      },
    });
  }, [eventHandler]);

  return (
    <Link to={descriptor.to} className={tileClassName} onClickCapture={handlePlayClick}>
      <Flex centered="axis" gap="large">
        <div className={s.icon}>
          {Icons.play}
        </div>

        <div className={s.title}>
          <span>{$L('#BottomNav_BrowseServers')}</span>
        </div>
      </Flex>
    </Link>
  );
});

function useHasLocalhostServer() {
  const LocalhostServer = useService(MpMenuLocalhostServerService);

  return !!LocalhostServer.address;
}

const LocalhostTile = observer(function LocalhostTile() {
  const LocalhostServer = useService(MpMenuLocalhostServerService);
  const ServersConnectService = useService(IServersConnectService);

  const computerName = useStreamerMode()
    ? '<HIDDEN>'
    : LocalhostServer.displayName;

  if (!LocalhostServer.address) {
    return null;
  }

  const handleClick = () => {
    ServersConnectService.connectTo(LocalhostServer.address);
  };

  const tileClassName = clsx(s.tile, s.tileLocalhost);

  return (
    <Interactive className={tileClassName} onClick={handleClick}>
      <div className={s.icon}>
        <FaServer />
      </div>

      <Text truncated size="normal" weight="bold" opacity="75">
        {$L('#Home_LocalServer_Title', { name: computerName })}
      </Text>
    </Interactive>
  );
});
