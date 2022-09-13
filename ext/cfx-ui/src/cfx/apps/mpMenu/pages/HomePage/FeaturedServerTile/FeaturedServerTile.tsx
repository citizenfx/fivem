import { MpMenuServersService } from "cfx/apps/mpMenu/services/servers/servers.mpMenu";
import { GameName } from "cfx/base/game";
import { currentGameNameIs } from "cfx/base/gameName";
import { useService } from "cfx/base/servicesContainer";
import { ServerTileItem } from "cfx/common/parts/Server/ServerTileItem/ServerTileItem";
import { $L } from "cfx/common/services/intl/l10n";
import { Icon } from "cfx/ui/Icon/Icon";
import { Icons } from "cfx/ui/Icons";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";

export const FeaturedServerTile = observer(function FeaturedServerTile() {
  const server = useFeaturedServer();
  if (!server) {
    return null;
  }

  const label = (
    <Flex centered="axis" gap="small">
      {/* <Icon opacity="50">
        {Icons.serversStaffPickUnstyled}
      </Icon> */}

      <Text opacity="50">
        {$L('#Home_PromotedServer_Ad')}
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

function useFeaturedServer() {
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
