import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";
import { $L } from "cfx/common/services/intl/l10n";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";

export const LastConnectedTile = observer(function LastConnectedTile() {
  const server = useLastHistoryServer();
  if (!server) {
    return null;
  }

  const label = (
    <Text opacity="50">
      {$L('#Home_LastConnectedServer')}
    </Text>
  );

  return (
    <ServerTileItem
      hideBanner
      label={label}
      server={server}
    />
  );
});

function useLastHistoryServer() {
  const ServersService = useServersService();

  const HistoryServerList = ServersService.getHistoryList();
  if (!HistoryServerList) {
    return null;
  }

  const serverId = HistoryServerList.sequence[0];
  if (!serverId) {
    return null;
  }

  const server = ServersService.getServer(serverId);
  if (!server) {
    return null;
  }

  return server;
}
