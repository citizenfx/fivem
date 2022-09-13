import { MpMenuLocalhostServerService } from "cfx/apps/mpMenu/services/servers/localhostServer.mpMenu";
import { MpMenuServersService } from "cfx/apps/mpMenu/services/servers/servers.mpMenu";
import { useService } from "cfx/base/servicesContainer";
import { SERVER_LIST_DESCRIPTORS } from "cfx/common/pages/ServersPage/ListTypeTabs";
import { $L } from "cfx/common/services/intl/l10n";
import { IServersConnectService } from "cfx/common/services/servers/serversConnect.service";
import { Tabular } from "cfx/ui/Tabular/Tabular";
import { ui } from "cfx/ui/ui";
import { observer } from "mobx-react-lite";
import { FaServer } from "react-icons/fa";
import { useNavigate } from "react-router-dom";

export const ServersNavBar = observer(function ServersNavBar() {
  const navigate = useNavigate();

  const ServersService = useService(MpMenuServersService);
  const LocalhostServer = useService(MpMenuLocalhostServerService);
  const ServersConnectService = useService(IServersConnectService);

  const content = (
    <Tabular.Root size="large" className={ui.cls.flexNoShrink}>
      {ServersService.listTypes.map((serverListType) => {
        const descriptor = SERVER_LIST_DESCRIPTORS[serverListType];
        if (!descriptor) {
          return null;
        }

        const isAllList = descriptor.to === '/servers';

        const label = isAllList
          ? $L(
            ServersService.totalServersCount
              ? '#Home_AllList_Link'
              : '#Home_AllList_Fallback',
            { count: ServersService.totalServersCount },
          )
          : descriptor.title;

        return (
          <Tabular.Item
            key={descriptor.to}
            icon={descriptor.icon}
            label={label}
            onClick={() => navigate(descriptor.to)}
          />
        );
      })}

      {!!LocalhostServer.address && (
        <Tabular.Item
          icon={<FaServer />}
          label={$L('#Home_LocalServer_Title', { name: LocalhostServer.displayName })}
          onClick={() => ServersConnectService.connectTo(LocalhostServer.address)}
        />
      )}
    </Tabular.Root>
  );

  return content;
});
