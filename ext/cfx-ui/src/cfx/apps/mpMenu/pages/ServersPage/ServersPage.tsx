import { ListTypeTabs } from "cfx/common/pages/ServersPage/ListTypeTabs";
import { ServersPage } from "cfx/common/pages/ServersPage/ServersPage";
import { useService } from "cfx/base/servicesContainer";
import { ServersListType } from "cfx/common/services/servers/lists/types";
import { IServersService } from "cfx/common/services/servers/servers.service";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { InsideNavBar } from "../../parts/NavBar/InsideNavBar";
import { Navigate } from "react-router-dom";
import { ServerFiltersWithDirectConnect } from "./ServerFiltersWithDirectConnect/ServerFiltersWithDirectConnect";
import { currentGameNameIs } from "cfx/base/gameName";
import { GameName } from "cfx/base/game";

export interface ServersPageProps {
  listType: ServersListType,
}
export const MpMenuServersPage = observer(function ({ listType }: ServersPageProps) {
  const ServersService = useService(IServersService);

  const serversList = ServersService.getList(listType);
  if (!serversList) {
    return (
      <Navigate to="/servers" />
    );
  }

  const config = serversList.getConfig?.();
  const showPinned = currentGameNameIs(GameName.FiveM) && listType === ServersListType.All;

  return (
    <>
      <InsideNavBar>
        <Flex>
          <ListTypeTabs />

          {!!config && (
            <>
              <ServerFiltersWithDirectConnect config={config} />

              {showPinned && (
                <div
                  style={{
                    width: 'calc(var(--width) * .293)'
                  }}
                />
              )}
            </>
          )}
        </Flex>
      </InsideNavBar>

      <ServersPage
        list={serversList}
        showPinned={showPinned}
      />
    </>
  );
});
