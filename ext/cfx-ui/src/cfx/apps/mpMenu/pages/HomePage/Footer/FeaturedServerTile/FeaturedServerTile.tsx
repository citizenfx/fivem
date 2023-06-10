import { MpMenuServersService } from "cfx/apps/mpMenu/services/servers/servers.mpMenu";
import { GameName } from "cfx/base/game";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { useService } from "cfx/base/servicesContainer";
import { ServerSelectorTileItem } from "cfx/common/parts/Server/ServerSelectorTileItem/ServerSelectorTileItem";
import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";
import { $L } from "cfx/common/services/intl/l10n";
import { useServersService } from "cfx/common/services/servers/servers.service";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";

export const FeaturedServerTile = observer(function FeaturedServerTile() {
  const ServersService = useServersService();

  const featuredServer = useFeaturedServer();
  if (!featuredServer) {
    return null;
  }

  const label = (
    <Text opacity="50">
      {$L('#Server_FeaturedServer_Title')}
    </Text>
  );

  if (featuredServer.type === 'collection') {
    return (
      <ServerSelectorTileItem
        label={label}
        serversCollection={featuredServer.collection}
      />
    );
  }

  const server = ServersService.getServer(featuredServer.id);
  if (!server) {
    return null;
  }

  return (
    <ServerTileItem
      hideBanner
      label={label}
      server={server}
    />
  );
});

export function useFeaturedServer() {
  const ServersService = useService(MpMenuServersService);

  if (!currentGameNameIs(GameName.FiveM)) {
    return null;
  }

  return ServersService.pinnedServersConfig?.featuredServer;
}
