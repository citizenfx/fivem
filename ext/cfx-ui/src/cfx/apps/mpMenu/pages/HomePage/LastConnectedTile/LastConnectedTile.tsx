import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { Icon } from "cfx/ui/Icon/Icon";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";

export const LastConnectedTile = observer(function LastConnectedTile() {
  // return null;
  const server = useLastHistoryServer();
  if (!server) {
    return null;
  }

  const label = (
    <Flex centered="axis" gap="small">
      {/* <Icon opacity="50">
        {Icons.serverLastConnected}
      </Icon> */}

      <Text opacity="50">
        Last connected server
      </Text>
    </Flex>
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
