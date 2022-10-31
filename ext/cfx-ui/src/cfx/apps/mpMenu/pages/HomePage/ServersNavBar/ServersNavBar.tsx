import { MpMenuServersService } from "cfx/apps/mpMenu/services/servers/servers.mpMenu";
import { useService } from "cfx/base/servicesContainer";
import { SERVER_LIST_DESCRIPTORS } from "cfx/common/pages/ServersPage/ListTypeTabs";
import { Tabular } from "cfx/ui/Tabular/Tabular";
import { ui } from "cfx/ui/ui";
import { observer } from "mobx-react-lite";
import { useNavigate } from "react-router-dom";

export const ServersNavBar = observer(function ServersNavBar() {
  const navigate = useNavigate();

  const ServersService = useService(MpMenuServersService);

  const content = (
    <Tabular.Root size="large" className={ui.cls.flexNoShrink}>
      {ServersService.listTypes.map((serverListType) => {
        const descriptor = SERVER_LIST_DESCRIPTORS[serverListType];
        if (!descriptor) {
          return null;
        }

        return (
          <Tabular.Item
            key={descriptor.to}
            icon={descriptor.icon}
            label={descriptor.title}
            onClick={() => navigate(descriptor.to)}
          />
        );
      })}
    </Tabular.Root>
  );

  return content;
});
