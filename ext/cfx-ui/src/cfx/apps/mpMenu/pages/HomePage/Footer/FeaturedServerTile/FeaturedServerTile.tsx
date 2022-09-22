import { MpMenuServersService } from "cfx/apps/mpMenu/services/servers/servers.mpMenu";
import { GameName } from "cfx/base/game";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { useService } from "cfx/base/servicesContainer";
import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";
import { $L } from "cfx/common/services/intl/l10n";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";

export const FeaturedServerTile = observer(function FeaturedServerTile() {
  const server = useFeaturedServer();
  if (!server) {
    return null;
  }

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
