import { MpMenuServersService } from "cfx/apps/mpMenu/services/servers/servers.mpMenu";
import { GameName } from "cfx/base/game";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { useService } from "cfx/base/servicesContainer";
import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";
import { $L } from "cfx/common/services/intl/l10n";
import { IServer, IServerView } from "cfx/common/services/servers/types";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";

export const FeaturedServerTile = observer(function FeaturedServerTile() {
  const server = useFeaturedServer();
  if (!server) {
    return null;
  }

  const serverInstances = getFeaturedServerInstances();

  const label = (
    <Text opacity="50">
      {$L('#Server_FeaturedServer_Title')}
    </Text>
  );

  return (
    <ServerTileItem
      hideBanner
      label={label}
      server={server}
      serverOtherInstances={serverInstances}
    />
  );
});

export function useFeaturedServer() {
  const ServersService = useService(MpMenuServersService);

  if (!currentGameNameIs(GameName.FiveM)) {
    return null;
  }

  const serverId = ServersService.pinnedServersConfig?.noAdServerId;
  if (!serverId) {
    return null;
  }

  return ServersService.getServer(serverId);
}

function getFeaturedServerInstances() {
  const ServersService = useService(MpMenuServersService);

  if (!currentGameNameIs(GameName.FiveM)) {
    return null;
  }

  const serverInstanceIds = ServersService.pinnedServersConfig?.noAdServerOtherInstanceIds;
  if (!serverInstanceIds || serverInstanceIds.length < 1) {
    return null;
  }

  const serverInstances:IServerView[] = [];
  for (const serverInstanceId in serverInstanceIds) {
    const serverInstance = ServersService.getServer(serverInstanceId);

    if (serverInstance) {
      serverInstances.push(serverInstance);
    }
  }

  return serverInstances;
}
