import { useStreamerMode } from "cfx/apps/mpMenu/services/convars/convars.service";
import { MpMenuLocalhostServerService } from "cfx/apps/mpMenu/services/servers/localhostServer.mpMenu";
import { useService } from "cfx/base/servicesContainer";
import { SERVER_LIST_DESCRIPTORS } from "cfx/common/pages/ServersPage/ListTypeTabs";
import { $L } from "cfx/common/services/intl/l10n";
import { ServersListType } from "cfx/common/services/servers/lists/types";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { IServersConnectService } from "cfx/common/services/servers/serversConnect.service";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { clsx } from "cfx/utils/clsx";
import { observer } from "mobx-react-lite";
import { BsPlayFill } from "react-icons/bs";
import { FaServer } from "react-icons/fa";
import { Link } from "react-router-dom";
import { LastConnectedTile, useLastHistoryServer } from "./LastConnectedTile/LastConnectedTile";
import s from './Continuity.module.scss';

export const Continuity = observer(function Continuity() {
  const hasLastConnectedServer = Boolean(useLastHistoryServer());
  const hasLocalhostServer = useHasLocalhostServer();

  const rootClassName = clsx(s.root, {
    [s.withLast]: hasLastConnectedServer,
    [s.withLocalhost]: hasLocalhostServer,
  })

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

      <SupportersTile />
      <FavoritesTile />
      <HistoryTile />
    </div>
  );
});

function formatServersCount(count: number): string {
  if (count < 1000) {
    return count.toString(10);
  }

  const thousands = (count / 1000) | 0;
  const hundreds = count - (thousands * 1000);

  return `${thousands},${hundreds.toString().padStart(3, '0')}`;
}

const PlayTile = observer(function PlayTile() {
  const ServersService = useServersService();

  const list = ServersService.getList(ServersListType.All);
  if (!list) {
    return null;
  }

  const descriptor = SERVER_LIST_DESCRIPTORS[ServersListType.All];

  const tileClassName = clsx(s.tile, s.tilePlay);

  return (
    <Link to={descriptor.to} className={tileClassName}>
      <div className={s.icon}>
        <BsPlayFill />
      </div>

      <div className={s.icon2}>
        <BsPlayFill />
      </div>
      <div className={s.icon3}>
        <BsPlayFill />
      </div>

      <Flex vertical>
        <div className={s.title}>
          <span>
            {$L('#BottomNav_Play')}
          </span>
        </div>

        <div className={s.subtitle}>
          {$L('#Home_AllList_Link', { count: formatServersCount(ServersService.totalServersCount) })}
        </div>
      </Flex>
    </Link>
  );
});

const SupportersTile = observer(function SupportersTile() {
  const ServersService = useServersService();

  const list = ServersService.getList(ServersListType.Supporters);
  if (!list) {
    return null;
  }

  const descriptor = SERVER_LIST_DESCRIPTORS[ServersListType.Supporters];

  const tileClassName = clsx(s.tile, s.tileSupporters);

  return (
    <Link to={descriptor.to} className={tileClassName}>
      <div className={s.icon}>
        {descriptor.icon}
      </div>

      <div className={s.title}>
        {$L('#ServerList_Premium')}
      </div>
    </Link>
  );
});

const FavoritesTile = observer(function FavoriteTile() {
  const ServersService = useServersService();

  const list = ServersService.getList(ServersListType.Favorites);
  if (!list) {
    return null;
  }

  const descriptor = SERVER_LIST_DESCRIPTORS[ServersListType.Favorites];

  const tileClassName = clsx(s.tile, s.tileFavorites);

  return (
    <Link to={descriptor.to} className={tileClassName}>
      <div className={s.icon}>
        {descriptor.icon}
      </div>

      <div className={s.title}>
        {$L('#ServerList_Favorites')}
      </div>
    </Link>
  );
});

const HistoryTile = observer(function HistoryTile() {
  const ServersService = useServersService();

  const list = ServersService.getList(ServersListType.History);
  if (!list) {
    return null;
  }

  const descriptor = SERVER_LIST_DESCRIPTORS[ServersListType.History];

  const tileClassName = clsx(s.tile, s.tileHistory);

  return (
    <Link to={descriptor.to} className={tileClassName}>
      <div className={s.icon}>
        {descriptor.icon}
      </div>

      <div className={s.title}>
        {$L('#ServerList_History')}
      </div>
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
    <div className={tileClassName} onClick={handleClick}>
      <div className={s.icon}>
        <FaServer />
      </div>

      <Text truncated size="normal" weight="bold" opacity="75">
        {$L('#Home_LocalServer_Title', { name: computerName })}
      </Text>
    </div>
  );
});
